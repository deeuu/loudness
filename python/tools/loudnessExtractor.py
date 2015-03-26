import numpy as np
import loudness as ln
import matplotlib.pyplot as plt

class LoudnessExtractor:
    '''Convienience class for processing numpy arrays.
    This class can be used to extract the loudness of an input array using a
    dynamic loudness model.
    The input model should be configured but not initialised. Model
    initialisation will take place internally.
    '''

    def __init__(self, model, fs = 32000, nInputEars = 1):

        if not model.isDynamicModel():
            raise ValueError("Model must be dynamic.")

        self.model = model
        self.fs = int(fs)
        self.nInputEars = nInputEars
        self.timeStep = model.getTimeStep()
        self.hopSize = int(fs * self.timeStep)
        self.sampleDelay = 0
        self.inputBuf = ln.SignalBank()
        self.inputBuf.initialize(self.nInputEars, 1, self.hopSize, self.fs)
        if not self.model.initialize(self.inputBuf):
            raise ValueError("Problem initialising the model!")

        #Assume signal bank containing loudness is final module
        self.outputBank = self.model.getModelOutput()
        self.nChannels = self.outputBank.getNChannels()
        self.nOutputEars = self.outputBank.getNEars()
        self.nSamplesToPadStart = 0
        self.nSamplesToPadEnd = int(0.2*self.fs)#see below
        self.x = None
        self.processed = False
        self.loudness = None

    def process(self, inputSignal):
        ''' Process the numpy array 'inputSignal' using a dynamic loudness
        model.  For stereo signals, the input signal must have two dimensions
        with shape (nSamples x 2).  For monophonic signals, the input signal can
        be (nSamples x 1) or one dimensional.  
        '''
        self.nSamples = inputSignal.shape[0]
        if inputSignal.ndim > 1:
            if self.nInputEars != inputSignal.shape[1]:
                raise ValueError("Input signal does not have " + str(self.nInputEars) \
                    + " dimensions")
        elif self.nInputEars > 1:
            raise ValueError("Input should have " \
                + str(self.nInputEars) + " columns");

        #configure the number of output frames needed
        nOutputFrames = int(np.ceil(self.nSamples / float(self.hopSize)))

        #output loudness
        self.loudness = np.zeros((self.nOutputEars, nOutputFrames, self.nChannels))

        #Format for SignalBank
        self.inputSignal = inputSignal.reshape((self.nInputEars, 1, self.nSamples))

        #Pad end so that we can obtain analysis over the last sample, 0.2ms is more than enough
        self.inputSignal = np.concatenate((\
            np.zeros((self.nInputEars, 1, self.nSamplesToPadStart)), \
            self.inputSignal,\
            np.zeros((self.nInputEars, 1, self.nSamplesToPadEnd))), 2)

        processingFrame = 0
        outputFrame = 0
        #One process call every hop samples
        while(outputFrame < nOutputFrames):

            #Any overlap is generated c++ side, so just fill the input
            #SignalBank with new blocks
            startIdx = processingFrame*self.hopSize
            endIdx = startIdx + self.hopSize
            self.inputBuf.setSignals(self.inputSignal[:, :, startIdx:endIdx])

            #Process the input buffer
            self.model.process(self.inputBuf)

            #get output if output buffer is ready
            if self.outputBank.getTrig():
                self.loudness[:,outputFrame,:] = self.outputBank.getSignals()[:,:,0]
                outputFrame += 1
            processingFrame += 1

        #Full processing so clear internal states
        self.model.reset()
        #Processing flag
        self.processed = True

    def plotLoudness(self):
        if self.processed:
            time = np.arange(self.nSamples) / float(self.fs)
            frameTime = np.arange(self.loudness.shape[1]) * self.timeStep
            fig, (ax1, ax2) = plt.subplots(2,1, sharex = True)
            for ear in range(self.nInputEars):
                ax1.plot(time,\
                        self.inputSignal[ear, 0, self.nSamplesToPadStart:self.nSamplesToPadStart+self.nSamples])
            ax1.set_ylabel("Amplitude")
            for ear in range(self.nOutputEars):
                ax2.plot(frameTime, self.loudness[ear])
            ax2.set_ylabel("Loudness")
            ax2.set_xlabel("Time, seconds")
            plt.tight_layout()
            plt.xlim(0, time[-1])
            plt.show()

    def computeGlobalLoudness(self, startSeconds=0, endSeconds=None,
            feature='MEAN', inPhons=False):
        '''Compute the average loudness from startSeconds to endSeconds.
        Time points are based on nearest sample, so it is possible that sample points outside of
        this range are included.
        No bounds checking here.'''
        if self.processed:
            start = int(np.round(startSeconds/self.timeStep))
            end = endSeconds
            if end is not None:
                end = int(np.round(endSeconds/self.timeStep)+1)
            if feature=='MEAN':
                globalLoudness = np.mean(self.loudness[:,start:end], 1)
            elif feature=='MAX':
                globalLoudness = np.max(self.loudness[:,start:end], 1)
            elif feature=='N5':
                globalLoudness = np.percentile(self.loudness[:,start:end],0.95, axis=1)
            else:
                globalLoudness = feature(self.loudness[:,start:end], axis=1)
            if inPhons:
                globalLoudness = np.array([ln.soneToPhon(x,True) for x in globalLoudness])
            return globalLoudness
