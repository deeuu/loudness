import numpy as np
import loudness as ln
import matplotlib.pyplot as plt

class LoudnessExtractor:
    '''Convienience class for processing numpy arrays'''

    def __init__(self, model, fs=32000):

        if not model.isDynamicModel():
            raise ValueError("Model must be dynamic.")

        self.model = model
        self.fs = int(fs)
        self.timeStep = model.getTimeStep()
        self.hopSize = int(fs*self.timeStep)
        self.sampleDelay = 0
        self.inputBuf = ln.SignalBank()
        self.inputBuf.initialize(1, self.hopSize, self.fs)
        if not self.model.initialize(self.inputBuf):
            raise ValueError("Problem initialising the model!")
        #Assume signal bank containing loudness is final module
        moduleIndex = None
        if moduleIndex is None:
            moduleIndex = self.model.getNModules()-1
        self.outputBank = self.model.getModuleOutput(moduleIndex)
        self.numChannels = self.outputBank.getNChannels()
        self.numSamplesToPadStart = 0
        self.numSamplesToPadEnd = int(0.2*self.fs)#see below
        self.x = None
        self.processed = False
        self.loudness = None

    def process(self, signal):

        numOutputFrames = int(np.ceil(signal.size/float(self.hopSize)))
        self.loudness = np.zeros((self.numChannels, numOutputFrames))
        outputFrame = 0

        #FrameGenerator will place data in centre of the analysis window
        #So only need to padd end, 0.2ms is more than enough
        self.x = np.hstack((np.zeros(self.numSamplesToPadStart), signal,
            np.zeros(self.numSamplesToPadEnd)))
        self.signalSize = signal.size
        processingFrame = 0

        #One process call every hop samples
        while(outputFrame < numOutputFrames):
            #Overlap is generated c++ side, so just fill with new blocks
            startIdx = processingFrame*self.hopSize
            endIdx = startIdx + self.hopSize
            self.inputBuf.setSignal(0, self.x[startIdx:endIdx])
            self.model.process(self.inputBuf)

            #get output if bank is ready
            if self.outputBank.getTrig():
                for chn in range(self.numChannels):
                    self.loudness[chn, outputFrame] = self.outputBank.getSample(chn, 0)
                outputFrame += 1
            processingFrame += 1
        #Full processing so clear internal states
        self.model.reset()
        #Processing flag
        self.processed = True

    def plotLoudness(self):
        if self.processed:
            time = np.arange(self.signalSize) / float(self.fs)
            frameTime = np.arange(self.loudness.shape[1]) * self.timeStep
            fig, (ax1, ax2) = plt.subplots(2,1, sharex = True)
            ax1.plot(time,\
                    self.x[self.numSamplesToPadStart:self.numSamplesToPadStart+self.signalSize])
            ax1.set_ylabel("Amplitude, pa")
            ax2.plot(frameTime, self.loudness.T)
            ax2.set_ylabel("Loudness, sones")
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
