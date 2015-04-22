import os, pickle
import numpy as np
import loudness as ln
from sound import Sound
import matplotlib.pyplot as plt
import h5py

class LoudnessExtractor:
    '''Convienience class for processing numpy arrays.
    This class can be used to extract features of an input array using a
    dynamic loudness model.

    The input model should be configured but not initialised. Model
    initialisation will take place internally.

    The output is stored in a dictionary with keys corresponding to the features
    listed in `modelOutputsToExtract'.

    Processing frames are per hop size, which is dictated by the rate of the
    model. Frame times are relative to the first sample of the input signal. For
    example, consider a hop size of 48 @ fs = 48kHz, then the first output frame
    is available at time 0.001s. You can offset the frame times using self.frameTimeOffset.
    '''

    def __init__(self, model,
            fs = 32000, modelOutputsToExtract = [], nInputEars = 1):
        ''' 
        Model   : The input loudness model - must be dynamic.
        fs      : The sampling frequency
        nInputEars  : Number of ears used by the input array to be analysed.
        modelOutputsToExtract   : A list of features output by the model to
                                  extract.
        '''

        if not model.isDynamic():
            raise ValueError("Model must be dynamic.")

        if type(modelOutputsToExtract) is not list:
                modelOutputsToExtract = [modelOutputsToExtract]

        self.model = model
        self.fs = int(fs)
        self.nInputEars = nInputEars
        self.rate = model.getRate() #desired rate in Hz
        self.hopSize = int(round(fs / self.rate))
        self.timeStep = float(self.hopSize) / fs
        self.inputBuf = ln.SignalBank()
        self.inputBuf.initialize(self.nInputEars, 1, self.hopSize, self.fs)
        if not self.model.initialize(self.inputBuf):
            raise ValueError("Problem initialising the model!")

        self.outputBanks = []
        self.outputDict = {}
        self.modelOutputsToExtract = modelOutputsToExtract
        for name in modelOutputsToExtract:
            self.outputBanks.append(self.model.getOutputModuleSignalBank(name))
        self.nSamplesToPadStart = 0
        self.nSamplesToPadEnd = int(0.2*self.fs)#see below
        self.frameTimeOffset = 0
        self.x = None
        self.processed = False
        self.loudness = None
        self.globalLoudness = None

    def process(self, inputSignal):
        '''
        Process the numpy array 'inputSignal' using a dynamic loudness
        model.  For stereo signals, the input signal must have two dimensions
        with shape (nSamples x 2).  For monophonic signals, the input signal can
        be 2D, i.e. (nSamples x 1), or one dimensional.  
        '''
        #Input checks
        self.nSamples = inputSignal.shape[0]
        if inputSignal.ndim > 1:
            if self.nInputEars != inputSignal.shape[1]:
                raise ValueError("Input signal does not have the expected\
                number of dimensions ( " + str(self.nInputEars) + " )")
        elif self.nInputEars > 1:
            raise ValueError("Input should have " \
                + str(self.nInputEars) + " columns");

        #Format input for SignalBank
        self.inputSignal = inputSignal.T.reshape((self.nInputEars, 1, self.nSamples))

        '''Pad end so that we can obtain analysis over the last sample, 
        No neat way to do this at the moment so assume 0.2ms is enough.'''
        self.inputSignal = np.concatenate((\
            np.zeros((self.nInputEars, 1, self.nSamplesToPadStart)), \
            self.inputSignal,\
            np.zeros((self.nInputEars, 1, self.nSamplesToPadEnd))), 2)

        #configure the number of output frames needed
        nOutputFrames = int(np.ceil(self.inputSignal.size / float(self.hopSize)))
        self.outputDict['FrameTimes'] = self.frameTimeOffset + self.timeStep + \
                np.arange(nOutputFrames) * self.hopSize / float(self.fs)

        #outputs
        for i, name in enumerate(self.modelOutputsToExtract):
            self.outputDict[name] = np.zeros((\
                nOutputFrames,
                self.outputBanks[i].getNEars(),\
                self.outputBanks[i].getNChannels(),\
                self.outputBanks[i].getNSamples()))

        #One process call every hop samples
        for frame in range(nOutputFrames):

            #Any overlap is generated c++ side, so just fill the input
            #SignalBank with new blocks
            startIdx = frame*self.hopSize
            endIdx = startIdx + self.hopSize
            self.inputBuf.setSignals(self.inputSignal[:, :, startIdx:endIdx])

            #Process the input buffer
            self.model.process(self.inputBuf)

            #get output if output buffer is ready
            for i, name in enumerate(self.modelOutputsToExtract):
                self.outputDict[name][frame] = self.outputBanks[i].getSignals()

        #Processing complete so clear internal states
        self.model.reset()
        self.processed = True

    def plotLoudnessTimeSeries(self, modelOutputsToPlot):
        '''
        Plots the input waveform and the features listed in
        `modelOutputsToPlot'. These features should have been extracted when
        calling `process()' and should have one sample per ear and frame, i.e.
        each feature should be one vector of loudness values per ear.
        '''
        if self.processed:

            if type(modelOutputsToPlot) is not list:
                    modelOutputsToPlot = [modelOutputsToPlot]

            time = np.arange(self.nSamples) / float(self.fs)
            fig, (ax1, ax2) = plt.subplots(2, 1, sharex = True)

            for ear in range(self.nInputEars):
                ax1.plot(time,\
                        self.inputSignal[ear, 0,\
                            self.nSamplesToPadStart:self.nSamplesToPadStart+self.nSamples])
            ax1.set_ylabel("Amplitude")

            for name in modelOutputsToPlot:
                for ear in range(self.outputDict[name].shape[1]):
                    ax2.plot(self.outputDict['FrameTimes'],\
                            self.outputDict[name][:, ear, 0, 0].flatten())
            ax2.set_ylabel("Loudness")
            ax2.set_xlabel("Time, seconds")
            plt.tight_layout()
            plt.xlim(0, time[-1])
            plt.show()

    def saveLoudnessTimeSeriesToCSV(self, filename, modelOutputsToSave):
        '''
        Saves the the features listed in `modelOutputsToSave' to a CSV file.
        These features should have been extracted when calling `process()' and
        should have one sample per ear and frame, i.e.  each feature should be
        one vector of loudness values per ear.  
        '''
        if self.processed:

            if type(modelOutputsToSave) is not list:
                    modelOutputsToSave = [modelOutputsToSave]

            dataToSave = self.outputDict['FrameTimes'].reshape((-1, 1))
            header = ['FrameTimes']
            for name in modelOutputsToSave:
                header.append(',' + name)
                for ear in range(self.outputDict[name].shape[1]):
                    dataToSave = np.hstack((dataToSave, \
                        self.outputDict[name][:, ear, 0, 0].reshape(-1,1)))

            filename, ext = os.path.splitext(filename)
            np.savetxt(filename + '.csv', dataToSave,\
                delimiter = ',', header = ','.join(self.outputDict.keys()) , comments = "")

    def saveOutputToPickle(self, filename):
        '''
        Saves the complete output dictionary to a pickle file.
        '''
        if self.processed:
            filename, ext = os.path.splitext(filename)
            with open(filename + '.pickle', 'wb') as outfile:
                pickle.dump(self.outputDict, outfile, protocol=pickle.HIGHEST_PROTOCOL)

    def computeGlobalLoudnessFeatures(self, modelOutputs, startSeconds=0, endSeconds=None):
        '''
        Compute the average loudness from startSeconds to endSeconds.
        Time points are based on nearest sample, so it is possible that sample points outside of
        this range are included.
        Loudness features are stored in the output dictionary.
        '''
        if self.processed:

            if type(modelOutputs) is not list:
                    modelOutputs = [modelOutputs]

            start = int(np.round(startSeconds/self.timeStep))
            end = endSeconds
            if end is not None:
                end = int(np.round(endSeconds/self.timeStep)+1)
            for name in modelOutputs:
                self.outputDict['Features'] = {name : {}}
                loudnessVec = self.outputDict[name][start:end, :, 0, 0]
                self.outputDict['Features'][name]['Mean'] = \
                        np.mean(loudnessVec, 0)
                self.outputDict['Features'][name]['Median'] = \
                        np.median(loudnessVec, 0)
                self.outputDict['Features'][name]['Max'] = \
                        np.max(loudnessVec, 0)
                self.outputDict['Features'][name]['N5'] = \
                        np.percentile(loudnessVec, 95, 0)
                self.outputDict['Features'][name]['IQR'] = \
                        np.percentile(loudnessVec, 75, 0) - \
                        np.percentile(loudnessVec, 25, 0)

class BatchWavFileProcessor:
    """Class for processing multiple wav files using an input loudness model.

    All processing takes place on the c++ side.

    Make sure your desired features are configured before calling this function.
    As an example:

    import loudness as ln
    model = ln.DynamicLoudnessCH2012()
    model.setOutputsToAggregate(['ShortTermLoudness', 'LongTermLoudness'])

    processor = BatchWavFileProcessor(wavFileDirectory, hdf5Filename)
    processor.process(model)

    All wav files in the directory `wavFileDirectory' will be processed and
    processed data will be saved as `hdf5Filename'.
    """

    def __init__(self,
                 wavFileDirectory = "",
                 filename = ""):

        self.wavFileDirectory = os.path.abspath(wavFileDirectory)
        self.filename = os.path.abspath(filename)

        #Get a list of all wav files in this directory
        self.wavFiles = []
        for file in os.listdir(self.wavFileDirectory):
            if file.endswith(".wav"):
                self.wavFiles.append(file)
        self.wavFiles.sort() #sort to avoid platform specific order
        if not self.wavFiles:
            raise ValueError("No wav files found")
        
        self.frameTimeOffset = 0
        self.audioFilesHaveSameSpec = False
        self.numFramesToAppend = 0

    def process(self, model):

        h5File = h5py.File(self.filename, 'w')

        processor = ln.AudioFileProcessor(\
                self.wavFileDirectory + '/' + self.wavFiles[0])

        if self.audioFilesHaveSameSpec:
            processor.initialize(model)

        for wavFile in self.wavFiles:
            
            #Create new group
            wavFileGroup = h5File.create_group(wavFile)

            processor.loadNewAudioFile(\
                    self.wavFileDirectory + '/' + wavFile)
            if not self.audioFilesHaveSameSpec:
                processor.initialize(model)

            processor.appendNFrames(self.numFramesToAppend)

            #configure the number of output frames needed
            wavFileGroup.create_dataset('FrameTimes', \
                    data = self.frameTimeOffset \
                    + processor.getTimeStep() \
                    * np.arange(1, processor.getNFrames() + 1))

            #Processing
            print("Processing file %s ..." % wavFile)
            processor.processAllFrames(model)

            #get output
            outputNames = model.getOutputModulesToAggregate()
            for name in outputNames:
                bank = model.getOutputModuleSignalBank(name)
                wavFileGroup.create_dataset(name, data =
                        bank.getAggregatedSignals())
        h5File.close()
