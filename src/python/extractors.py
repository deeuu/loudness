import os, pickle
import numpy as np
import loudness as ln
from sound import Sound
import matplotlib.pyplot as plt
import h5py

class StationaryLoudnessExtractor:

    def __init__(self, model, outputs):

        if model.isDynamic():
            raise ValueError("Model cannot be dynamic")
        self.model = model
        if outputs is None:
            raise ValueError("Must specify outputs")
        if type(outputs) is not list:
            self.outputs = [outputs]
        else:
            self.outputs = outputs
        self.outputDict = {}
        self.outputDict['Frequencies'] = np.array([])
        self.nEars = 0
        self.bank = ln.SignalBank()
        self.initialize = True

    def process(self, frequencies, intensityLevels):

        if (frequencies.size == 0) or (intensityLevels.size == 0):
            raise ValueError("Must have at least one input component")

        if intensityLevels.ndim == 1:
            intensities = 10**(intensityLevels.reshape((-1, 1)) / 10.0)
        else:
            intensities = 10**(intensityLevels / 10.0)

        if intensities.shape[0] != frequencies.size:
            raise ValueError("Number of component intensities does not match"+
                " number of component frequencies")

        self.outputDict['IntensityLevels'] = np.squeeze(intensityLevels)

        nComponents = frequencies.size
        nEars = intensities.shape[1]

        if self.initialize:
            self.bank.initialize(nEars, nComponents, 1, 1)
            self.bank.setCentreFreqs(frequencies)
            self.outputDict['Frequencies'] = frequencies
            self.nEars = nEars
            self.model.initialize(self.bank)
        self.initialize = False

        self.bank.setSignals(intensities.reshape((nEars, nComponents, 1)))
        
        self.model.process(self.bank)

        for name in self.outputs:
            outputBank = self.model.getOutput(name)
            self.outputDict[name] = (np.squeeze(outputBank.getSignals())).copy()

        self.model.reset()

class DynamicLoudnessExtractor:
    '''Convienience class for processing numpy arrays.
    This class can be used to extract features of an input array using a
    dynamic loudness model.

    The input model should be configured but not initialised. Model
    initialisation will take place internally.

    The output is stored in a dictionary with keys corresponding to the features
    listed in `modelOutputsToExtract'.

    Processing frames are per hop size, which is determined by the rate of the
    model. Frame times start at time 0 and increment at the hop size in seconds.
    For example, consider a hop size of 48 @ fs = 48kHz, the frame time
    corresponding to the first and second frame would be 0 and 1ms respectively.
    You can offset the frame times using self.frameTimeOffset.  '''

    def __init__(self, 
            model,
            fs = 32000, 
            outputs = None,
            nInputEars = 1,
            numSecondsToPadStartBy = 0,
            numSecondsToPadEndBy = 0.2,
            frameTimeOffset = 0,
            gainInDecibels = 0.0):
        ''' 
        Model   
            The input loudness model - must be dynamic.
        fs   
            The sampling frequency
        nInputEars
            Number of ears used by the input array to be analysed.
        '''

        if not model.isDynamic():
            raise ValueError("Model must be dynamic.")

        self.model = model
        self.fs = int(fs)
        self.nInputEars = nInputEars
        self.rate = model.getRate() #desired rate in Hz
        self.hopSize = int(np.round(fs / self.rate))
        self.timeStep = float(self.hopSize) / fs
        self.inputBuf = ln.SignalBank()
        self.inputBuf.initialize(self.nInputEars, 1, self.hopSize, self.fs)
        self.outputs = outputs
        self.gainInDecibels = gainInDecibels
        if self.outputs is not None:
            if type(self.outputs) is not list:
                self.outputs = [self.outputs]
            self.model.setOutputsToAggregate(self.outputs)
        else:
            raise ValueError("Must specify outputs")

        if not self.model.initialize(self.inputBuf):
            raise ValueError("Problem initialising the model!")

        self.outputDict = {}
        self.nSamplesToPadStart = np.round(numSecondsToPadStartBy * self.fs)
        self.nSamplesToPadEnd = np.round(numSecondsToPadEndBy * self.fs)
        self.frameTimeOffset = frameTimeOffset
        self.x = None
        self.processed = False
        self.loudness = None
        self.globalLoudness = None

    def process(self, inputSignal):
        '''
        Process the numpy array `inputSignal' using a dynamic loudness
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
        self.inputSignal = np.concatenate((
            np.zeros((self.nInputEars, 1, self.nSamplesToPadStart)),
            self.inputSignal,
            np.zeros((self.nInputEars, 1, self.nSamplesToPadEnd))), 2)

        # Apply any gain
        self.inputSignal *= 10 ** (self.gainInDecibels / 20.0)

        #configure the number of output frames needed
        nOutputFrames = int(np.ceil(self.inputSignal.shape[2] / float(self.hopSize)))
        self.outputDict['FrameTimes'] = (self.frameTimeOffset +
                                        np.arange(nOutputFrames) *
                                        self.hopSize / float(self.fs))

        #One process call every hop samples
        for frame in range(nOutputFrames):

            #Any overlap is generated c++ side, so just fill the input
            #SignalBank with new blocks
            startIdx = frame * self.hopSize
            endIdx = startIdx + self.hopSize
            self.inputBuf.setSignals(self.inputSignal[:, :, startIdx:endIdx])

            #Process the input buffer
            self.model.process(self.inputBuf)

        #get outputs
        for name in self.outputs:
            bank = self.model.getOutput(name)
            self.outputDict[name] = np.squeeze(bank.getAggregatedSignals())
            if self.outputDict[name].ndim == 1:
                self.outputDict[name] = self.outputDict[name].reshape((-1,1))

        #Processing complete so clear internal states
        self.model.reset()
        self.processed = True

    def saveOutputToPickle(self, filename):
        '''
        Saves the complete output dictionary to a pickle file.
        '''
        if self.processed:
            print 'Saving to pickle file'
            filename, ext = os.path.splitext(filename)
            with open(filename + '.pickle', 'wb') as outfile:
                pickle.dump(self.outputDict, outfile, protocol=pickle.HIGHEST_PROTOCOL)

class BatchWavFileProcessor:
    """Class for processing multiple wav files using a given loudness model.

    All processing takes place on the c++ side.

    Make sure your desired features are configured before calling this function.
    As an example:

    import loudness as ln
    model = ln.DynamicLoudnessCH2012()

    processor = BatchWavFileProcessor(wavFileDirectory, hdf5Filename, ['ShortTermLoudness', 'LongTermLoudness'])
    processor.process(model)

    All wav files in the directory `wavFileDirectory' will be processed and
    data will be saved as `hdf5Filename' (a hdf5 file).
    
    Processing frames are per hop size, which is dictated by the rate of the
    model. Frame times start at time 0 and increment at the hop size in seconds.
    For example, consider a hop size of 48 @ fs = 48kHz, the frame time
    corresponding to the first and second frame would be 0 and 1ms respectively.
    You can offset the frame times using self.frameTimeOffset.  '''
    """

    def __init__(self,
                 wavFileDirectory = "",
                 filename = "",
                 outputs = None,
                 numFramesToAppend = 0,
                 frameTimeOffset = 0,
                 audioFilesHaveSameSpec = False):

        self.filename = os.path.abspath(filename)
        self.wavFileDirectory = os.path.dirname(wavFileDirectory)
        if wavFileDirectory.endswith('.wav'):
            self.wavFiles = [os.path.basename(wavFileDirectory)]
        else:
            #Get a list of all wav files in this directory
            self.wavFiles = []
            for file in os.listdir(self.wavFileDirectory):
                if file.endswith(".wav"):
                    self.wavFiles.append(file)
            self.wavFiles.sort() #sort to avoid platform specific order
            if not self.wavFiles:
                raise ValueError("No wav files found")
        
        self.frameTimeOffset = frameTimeOffset
        self.audioFilesHaveSameSpec = audioFilesHaveSameSpec
        self.numFramesToAppend = numFramesToAppend
        self.gainInDecibels = gainInDecibels

        self.outputs = outputs
        if self.outputs is not None:
            if type(outputs) is not list:
                self.outputs = [outputs]
        else:
            raise ValueError("No outputs specified.")

    def process(self, model):

        model.setOutputsToAggregate(self.outputs)
        
        print "Output will be saved to ", self.filename
        h5File = h5py.File(self.filename, 'w')

        processor = ln.AudioFileProcessor(\
                self.wavFileDirectory + '/' + self.wavFiles[0])

        if self.audioFilesHaveSameSpec:
            processor.initialize(model)

        for wavFile in self.wavFiles:
            
            print("Processing file %s ..." % wavFile)

            #Create new group
            wavFileGroup = h5File.create_group(wavFile)

            processor.setGainInDecibels(self.gainInDecibels)
            processor.loadNewAudioFile(\
                    self.wavFileDirectory + '/' + wavFile)
            if not self.audioFilesHaveSameSpec:
                processor.initialize(model)

            processor.appendNFrames(self.numFramesToAppend)

            #configure the number of output frames needed
            wavFileGroup.create_dataset('FrameTimes',\
                    data = self.frameTimeOffset +
                    np.arange(processor.getNFrames()) * processor.getTimeStep())

            #Processing
            processor.processAllFrames(model)

            #get output
            for name in self.outputs:
                bank = model.getOutput(name)
                wavFileGroup.create_dataset(name, data =
                        np.squeeze(bank.getAggregatedSignals()))
        h5File.close()
