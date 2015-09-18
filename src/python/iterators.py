import numpy as np
import loudness as ln
import matplotlib.pyplot as plt
from sound import Sound
from extractors import DynamicLoudnessExtractor, StationaryLoudnessExtractor

def asIs(x):
    return x

class DynamicLoudnessIterator():
    
    def __init__(self, 
            model, 
            fs,
            output = None,
            globalLoudnessFeature = None,
            loudnessLevelFunction = None,
            nInputEars = 1):

        self.extractor = DynamicLoudnessExtractor(model, fs, nInputEars, output)
        self.outputName = output
        self.converged = False

        if globalLoudnessFeature is None:
            self.globalLoudnessFeature = np.mean
        else:
            self.globalLoudnessFeature = globalLoudnessFeature
        
        if loudnessLevelFunction is None:
            self.loudnessLevelFunction = asIs
        else:
            self.loudnessLevelFunction = loudnessLevelFunction

    def extractLoudness(self, signal, gainInDecibels = 0):
        signalIn = signal.copy() * 10 ** (gainInDecibels / 20.0)
        self.extractor.process(signalIn)
        timeSeries = self.extractor.outputDict[self.outputName]
        loudness = self.globalLoudnessFeature(timeSeries)
        print loudness
        loudnessLevel = self.loudnessLevelFunction(loudness)
        return loudnessLevel

    def process(self, inputSignal, targetLoudness, tol = 0.1, nIters = 5, alpha = 1.0):

        # First check if the target loudness is a signal
        if type(targetLoudness) is np.ndarray:
            targetLoudness = self.extractLoudness(targetLoudness)
        elif targetLoudness is None:
            raise ValueError('Target loudness must be specified')

        storedGain = 0

        self.converged = False
        for i in range(nIters):

            loudnessLevel = self.extractLoudness(inputSignal, storedGain)

            error = targetLoudness - loudnessLevel

            print (('Gain: %0.3f, Loudness Level: %0.3f, ' +
                    'Error: %0.3f') % (storedGain, loudnessLevel, error))

            if np.abs(error) < tol:
                self.converged = True
                break
            else:
                storedGain += error * alpha
                if (i == (nIters-1)):
                    print "Reached iteration limit, not solved within desired error tolerance."

        return storedGain

class StationaryLoudnessIterator():

    def __init__(self, model, outputName, loudnessLevelFunction = None):
        
        self.outputName = outputName
        self.extractor = StationaryLoudnessExtractor(model, outputName)
        self.converged = False

        if loudnessLevelFunction is None:
            self.loudnessLevelFunction = asIs
        else:
            self.loudnessLevelFunction = loudnessLevelFunction

    def extractLoudness(self, frequencies, intensityLevels, gainInDecibels = 0):
        self.extractor.process(frequencies, intensityLevels + gainInDecibels)
        loudness = self.extractor.outputDict[self.outputName]
        loudnessLevel = self.loudnessLevelFunction(loudness)
        return loudnessLevel

    def process(self, frequencies, intensityLevels,
            targetLoudnessFrequencies, 
            targetLoudnessIntensityLevels, 
            tol, 
            nIters = 5,
            alpha = 1.0):

        if type(targetLoudnessIntensityLevels) is np.ndarray:
            targetLoudness = self.extractLoudness(targetLoudnessFrequencies,
                    targetLoudnessIntensityLevels)
        else:
            targetLoudness = targetLoudnessIntensityLevels

        storedGain = 0

        self.converged = False
        for i in range(nIters):

            loudnessLevel = self.extractLoudness(frequencies, intensityLevels, storedGain)

            error = targetLoudness - loudnessLevel

            print (('Gain: %0.3f, Loudness Level: %0.3f, ' +
                    'Error: %0.3f') % (storedGain, loudnessLevel, error))

            if np.abs(error) < tol:
                self.converged = True
                break
            else:
                storedGain += error * alpha
                if (i == (nIters-1)):
                    print "Reached iteration limit, not solved within desired error tolerance."

        return storedGain

class BatchWavFileIterator:
    '''
    A wrapper around DynamicLoudnessIterator for batch processing audio files.
    This class can be used to search for the gains required to achieve a target
    loudness for multiple wav files.
    '''

    def __init__(self,
                 wavFileDirectory = "",
                 output = None):

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
        
        self.numFramesToAppend = 0
        self.gainInDecibels = 0
        self.targetLoudness = None
        self.globalLoudnessFeature = None
        self.loudnessLevelFunction = None
        self.tol = 0.1
        self.nIters = 5
        self.alpha = 1.0

        self.output = output
        if self.output is None or type(self.output) is list:
            raise ValueError("Only one model output allowed.")

    def process(self, model):

        self.gains = {}

        for wavFile in self.wavFiles:
            
            # Load the audio file and configure level
            sound = Sound.readFromAudioFile(
                    self.wavFileDirectory + '/' + wavFile)
            sound.useDBSPL()
            sound.applyGain(self.gainInDecibels)

            # Instantiate a dynamic loudness iterator
            processor = DynamicLoudnessIterator(
                    model, 
                    sound.fs, 
                    self.output,
                    self.globalLoudnessFeature,
                    self.loudnessLevelFunction,
                    sound.nChannels)

            # Process the audio file
            print("Processing file %s ..." % wavFile)

            gain = processor.process(sound.data, 
                    self.targetLoudness,
                    self.tol,
                    self.nIters,
                    self.alpha)

            # Get gain required for target loudness
            self.gains[wavFile] = gain 

        return self.gains

class StationaryLoudnessISOThresholdPredictor():

    def __init__(self, model, outputName, loudnessLevelFunction = None):

        self.iterator = StationaryLoudnessIterator(model, outputName, loudnessLevelFunction)

        #ISO data
        self.freqsISO = np.array([20.0, 25, 31.5, 40, 50, 63, 80, 100, 125, 160,
            200, 250, 315, 400, 500, 630, 750, 800, 1000, 1250, 1500, 1600,
            2000, 2500, 3000, 3150, 4000, 5000, 6000, 6300, 8000, 9000, 10000,
            11200, 12500, 14000])

        self.thresholdsISO = np.array([78.5, 68.7, 59.5, 51.1, 44, 37.5, 31.5,
            26.5, 22.1, 17.9, 14.4, 11.4, 8.6, 6.2, 4.4, 3, 2.4, 2.2, 2.4, 3.5,
            2.4, 1.7, -1.3, -4.2, -5.8, -6.0, -5.4, -1.5, 4.3, 6, 12.6, 13.9,
            13.9, 13, 12.3, 18.4])

        self.threshold = 2.2
        self.tol = 0.01
        self.nIters = 10
        self.predictions = None

    def process(self):

        self.predictions = np.zeros(self.freqsISO.size)
        for i, freq in enumerate(self.freqsISO):
            levels = np.zeros(self.freqsISO.size) - 100
            levels[i] = self.thresholdsISO[i]
            self.predictions[i] = self.thresholdsISO[i]
            self.predictions[i] += self.iterator.process(self.freqsISO,
                    levels, None,
                    self.threshold, self.tol, self.nIters)

    def plotPredictions(self):

        plt.semilogx(self.freqsISO, self.thresholdsISO, label = 'ISO')
        plt.semilogx(self.freqsISO, self.predictions, color = 'r', 
                linestyle = '--', label = 'Predicted')
        plt.legend()
        plt.show()

class DynamicLoudnessISOThresholdPredictor():

    def __init__(self, model, fs, outputName,
            globalLoudnessFeature = None,
            loudnessLevelFunction = None):

        self.iterator = DynamicLoudnessIterator(model, fs, outputName, 
                globalLoudnessFeature,
                loudnessLevelFunction)

        #ISO data
        self.freqsISO = np.array([20.0, 25, 31.5, 40, 50, 63, 80, 100, 125, 160,
            200, 250, 315, 400, 500, 630, 750, 800, 1000, 1250, 1500, 1600,
            2000, 2500, 3000, 3150, 4000, 5000, 6000, 6300, 8000, 9000, 10000,
            11200, 12500, 14000])

        self.thresholdsISO = np.array([78.5, 68.7, 59.5, 51.1, 44, 37.5, 31.5,
            26.5, 22.1, 17.9, 14.4, 11.4, 8.6, 6.2, 4.4, 3, 2.4, 2.2, 2.4, 3.5,
            2.4, 1.7, -1.3, -4.2, -5.8, -6.0, -5.4, -1.5, 4.3, 6, 12.6, 13.9,
            13.9, 13, 12.3, 18.4])

        self.threshold = 2.2
        self.tol = 0.01
        self.nIters = 10
        self.predictions = None
        self.fs = fs
        self.duration = 1

    def process(self):

        self.predictions = np.zeros(self.freqsISO.size)
        for i, freq in enumerate(self.freqsISO):

            s = Sound.tone([freq], dur = self.duration, fs = self.fs)
            s.applyRamp(0.1)
            s.useDBSPL()
            s.normalise(self.thresholdsISO[i], 'RMS')

            self.predictions[i] = self.thresholdsISO[i]
            self.predictions[i] += self.iterator.process(s.data,
                    self.threshold, self.tol, self.nIters)

    def plotPredictions(self):

        plt.semilogx(self.freqsISO, self.thresholdsISO, label = 'ISO')
        plt.semilogx(self.freqsISO, self.predictions, color = 'r', 
                linestyle = '--', label = 'Predicted')
        plt.legend()
        plt.show()
