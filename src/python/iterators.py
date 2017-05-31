import os
import numpy as np
import matplotlib.pyplot as plt
from .sound import Sound
from .extractors import DynamicLoudnessExtractor, StationaryLoudnessExtractor


def asIs(x):
    return x


def powerToDecibels(power):
    return 10 * np.log10(power + 1e-10)


def meanSoneToDecibels(sone):
    return powerToDecibels(np.mean(sone[sone > 0.0]))


def maxSoneToDecibels(sone):
    return powerToDecibels(np.max(sone[sone > 0.0]))


class DynamicLoudnessIterator():

    def __init__(self,
                 model,
                 fs,
                 outputName=None,
                 loudnessFunction=None,
                 nInputEars=1,
                 tol=0.1,
                 nIters=10,
                 alpha=1.0,
                 nSecondsToPadStartBy=0.0,
                 nSecondsToPadEndBy=0.2,
                 printResults=False):

        if type(outputName) is list:
            outputName = outputName[0]

        self.extractor = DynamicLoudnessExtractor(model,
                                                  fs,
                                                  outputName,
                                                  1,
                                                  nInputEars,
                                                  nSecondsToPadStartBy,
                                                  nSecondsToPadEndBy)
        self.outputName = outputName
        self.converged = False
        self.tol = tol
        self.nIters = nIters
        self.alpha = alpha
        self.printResults = printResults

        if loudnessFunction is None:
            self.loudnessFunction = np.mean
        else:
            self.loudnessFunction = loudnessFunction

    def extractLoudness(self, signal, gainInDecibels=0.0):

        gain = 10 ** (gainInDecibels / 20.0)
        out = self.extractor.process(signal * gain)
        timeSeries = out[self.outputName]
        loudness = self.loudnessFunction(timeSeries)
        return loudness

    def process(self, inputSignal, targetLoudness):

        # First check if the target loudness is a signal
        if type(targetLoudness) is np.ndarray:
            targetLoudness = self.extractLoudness(targetLoudness)
        elif targetLoudness is None:
            raise ValueError('Target loudness must be specified')

        storedGain = 0.0

        self.converged = False
        for i in range(self.nIters):

            loudness = self.extractLoudness(inputSignal, storedGain)

            error = targetLoudness - loudness

            if self.printResults:
                print (('Gain: %0.3f, Loudness: %0.3f' +
                        ' Target: %0.3f, Error: %0.3f')
                       % (storedGain, loudness, targetLoudness, error))

            if np.abs(error) < self.tol:
                self.converged = True
                break
            else:
                storedGain += error * self.alpha
                if (i == (self.nIters-1)):
                    print ("Reached iteration limit, not solved " +
                           "within desired error tolerance.")

        return storedGain


class StationaryLoudnessIterator():

    def __init__(self,
                 model,
                 outputName=None,
                 loudnessFunction=None,
                 tol=0.1,
                 nIters=10,
                 alpha=1.0,
                 alwaysReinitialize=False):

        self.outputName = outputName
        self.extractor = StationaryLoudnessExtractor(model, outputName,
                                                     alwaysReinitialize)
        self.converged = False
        self.tol = tol
        self.nIters = nIters
        self.alpha = alpha
        self.printResults = False

        if loudnessFunction is None:
            self.loudnessFunction = asIs
        else:
            self.loudnessFunction = loudnessFunction

    def extractLoudness(self, frequencies, intensityLevels, gainInDecibels=0):
        self.extractor.process(frequencies, intensityLevels + gainInDecibels)
        loudness = self.extractor.outputDict[self.outputName]
        loudness = self.loudnessFunction(loudness)
        return loudness

    def process(self,
                frequencies,
                intensityLevels,
                targetLoudnessFrequencies,
                targetLoudnessIntensityLevels):

        if targetLoudnessFrequencies is not None:
            targetLoudness = self.extractLoudness(
                targetLoudnessFrequencies,
                targetLoudnessIntensityLevels
            )
        else:
            targetLoudness = targetLoudnessIntensityLevels

        storedGain = 0

        self.converged = False
        for i in range(self.nIters):

            loudness = self.extractLoudness(
                frequencies, intensityLevels, storedGain
            )
            error = targetLoudness - loudness

            if self.printResults:
                print (('Gain: %0.3f, Loudness: %0.3f, ' +
                        'Error: %0.3f') % (storedGain, loudness, error))

            if np.abs(error) < self.tol:
                self.converged = True
                break
            else:
                storedGain += error * self.alpha
                if (i == (self.nIters-1)):
                    print ("Reached iteration limit, not solved " +
                           "within desired error tolerance.")

        return storedGain


class BatchDynamicLoudnessIterator:
    '''
    A wrapper around DynamicLoudnessIterator for batch processing a list of
    signals. This class can be used to search for the gains required to achieve
    a target loudness.  Signals can have individual target loudness values.
    '''

    def __init__(self,
                 model,
                 fs,
                 output=None,
                 loudnessFunction=None,
                 tol=0.1,
                 nIters=10,
                 alpha=1.0,
                 nSecondsToPadStartBy=0.0,
                 nSecondsToPadEndBy=0.2,
                 printResults=False):

        self.nSecondsToPadStartBy = nSecondsToPadStartBy
        self.nSecondsToPadEndBy = nSecondsToPadEndBy
        self.loudnessFunction = loudnessFunction
        self.tol = tol
        self.nIters = nIters
        self.alpha = alpha
        self.model = model
        self.fs = fs
        self.printResults = printResults

        self.output = output
        if self.output is None or type(self.output) is list:
            raise ValueError("Only one model output allowed.")

    def process(self, listOfInputSignals, targetLoudness=None):
        print(self.printResults)

        hasMultipleTargets = False
        if type(targetLoudness) in [np.ndarray, list]:
            if len(targetLoudness) == len(listOfInputSignals):
                hasMultipleTargets = True

        gains = np.zeros(len(listOfInputSignals))

        for i, signal in enumerate(listOfInputSignals):

            if signal.ndim == 1:
                signal = signal.reshape((-1, 1))

            # Instantiate a dynamic loudness iterator
            processor = DynamicLoudnessIterator(
                    self.model,
                    self.fs,
                    self.output,
                    self.loudnessFunction,
                    signal.shape[1],
                    self.tol,
                    self.nIters,
                    self.alpha,
                    self.nSecondsToPadStartBy,
                    self.nSecondsToPadEndBy,
                    self.printResults)

            # Process the audio file
            print("Processing signal %d ..." % i)

            if hasMultipleTargets:
                target = targetLoudness[i]
            else:
                target = targetLoudness

            gain = processor.process(signal, target)

            # Get gain required for target loudness
            gains[i] = gain

        return gains


class EqualDynamicLoudnessIterator():
    '''
    This class can be used to find the gain required for equal loudness between
    two sounds. Method `process' expects a list of paired signals (tuples), in
    the form of (fixedStimulus, variableStimulus). The loudness of the fixed
    stimulus is extracted and the level of the variable stimulus is adjusted to
    approximate the point of equal loudness.  Multiple pairs can be passed to
    `process' but consider memory if the signals are very long or there are
    lots of them.
    '''

    def __init__(self,
                 model,
                 fs,
                 output=None,
                 loudnessFunction=None,
                 nInputEars=1,
                 tol=0.1,
                 nIters=10,
                 alpha=1.0,
                 nSecondsToPadStartBy=0.0,
                 nSecondsToPadEndBy=0.2):

        self.extractor = DynamicLoudnessIterator(model,
                                                 fs,
                                                 output,
                                                 loudnessFunction,
                                                 nInputEars,
                                                 tol,
                                                 nIters,
                                                 alpha,
                                                 nSecondsToPadStartBy,
                                                 nSecondsToPadEndBy)

    def process(self, pairs):

        gain = np.zeros(len(pairs))

        for i, pair in enumerate(pairs):

            print('Processing pair %d' % i)

            # Find gain required for equal loudness
            gain[i] = self.extractor.process(pair[1], pair[0])

        return gain


class BatchWavFileIterator:
    '''
    A wrapper around DynamicLoudnessIterator for batch processing audio files.
    This class can be used to search for the gains required to achieve a target
    loudness for multiple wav files.
    Audio files can have individual target loudness values.
    '''

    def __init__(self,
                 wavFileDirectory,
                 model,
                 output=None,
                 loudnessFunction=None,
                 targetLoudness=None,
                 tol=0.1,
                 nIters=10,
                 alpha=1.0,
                 nSecondsToPadStartBy=0,
                 nSecondsToPadEndBy=0.2,
                 gainInDecibels=0.0):

        self.wavFileDirectory = os.path.dirname(wavFileDirectory)
        if wavFileDirectory.endswith('.wav'):
            self.wavFiles = [os.path.basename(wavFileDirectory)]
        else:
            # Get a list of all wav files in this directory
            self.wavFiles = []
            for file in os.listdir(self.wavFileDirectory):
                if file.endswith(".wav"):
                    self.wavFiles.append(file)
            self.wavFiles.sort()  # Sort to avoid platform specific order
            if not self.wavFiles:
                raise ValueError("No wav files found")

        self.nSecondsToPadStartBy = nSecondsToPadStartBy
        self.nSecondsToPadEndBy = nSecondsToPadEndBy
        self.gainInDecibels = gainInDecibels
        self.targetLoudness = targetLoudness
        self.loudnessFunction = loudnessFunction
        self.tol = tol
        self.nIters = nIters
        self.alpha = alpha

        self.output = output
        if self.output is None or type(self.output) is list:
            raise ValueError("Only one model output allowed.")

    def process(self):

        hasMultipleTargets = False
        if type(self.targetLoudness) in [np.ndarray, list]:
            if len(self.targetLoudness) == len(self.wavFiles):
                hasMultipleTargets = True

        self.gains = {}

        for i, wavFile in enumerate(self.wavFiles):

            # Load the audio file and configure level
            sound = Sound.readFromAudioFile(
                    self.wavFileDirectory + '/' + wavFile)
            sound.applyGain(self.gainInDecibels)

            # Instantiate a dynamic loudness iterator
            processor = DynamicLoudnessIterator(self.model,
                                                sound.fs,
                                                self.output,
                                                self.loudnessFunction,
                                                sound.nChannels,
                                                self.tol,
                                                self.nIters,
                                                self.alpha,
                                                self.nSecondsToPadStartBy,
                                                self.nSecondsToPadEndBy)

            # Process the audio file
            print("Processing file %s ..." % wavFile)

            if hasMultipleTargets:
                target = self.targetLoudness[i]
            else:
                target = self.targetLoudness

            gain = processor.process(sound.data, target)

            # Get gain required for target loudness
            self.gains[wavFile] = gain

        return self.gains


class StationaryLoudnessISOThresholdPredictor():

    def __init__(self,
                 model,
                 outputName,
                 loudnessFunction=None,
                 tol=0.1,
                 nIters=10,
                 alpha=0.1,
                 threshold=2.2):

        self.iterator = StationaryLoudnessIterator(model,
                                                   outputName,
                                                   loudnessFunction,
                                                   tol,
                                                   nIters,
                                                   alpha)
        # ISO data
        self.freqsISO = np.array([
            20.0, 25, 31.5, 40, 50, 63, 80, 100, 125, 160,
            200, 250, 315, 400, 500, 630, 750, 800, 1000, 1250, 1500, 1600,
            2000, 2500, 3000, 3150, 4000, 5000, 6000, 6300, 8000, 9000, 10000,
            11200, 12500, 14000
        ])

        self.thresholdsISO = np.array([
            78.5, 68.7, 59.5, 51.1, 44, 37.5, 31.5,
            26.5, 22.1, 17.9, 14.4, 11.4, 8.6, 6.2, 4.4, 3, 2.4, 2.2, 2.4, 3.5,
            2.4, 1.7, -1.3, -4.2, -5.8, -6.0, -5.4, -1.5, 4.3, 6, 12.6, 13.9,
            13.9, 13, 12.3, 18.4
        ])

        self.threshold = threshold
        self.predictions = None

    def process(self):

        self.predictions = np.zeros(self.freqsISO.size)
        for i, freq in enumerate(self.freqsISO):
            levels = np.zeros(self.freqsISO.size) - 100
            levels[i] = self.thresholdsISO[i]
            self.predictions[i] = self.thresholdsISO[i]
            self.predictions[i] += self.iterator.process(self.freqsISO,
                                                         levels,
                                                         None)

    def plotPredictions(self):

        plt.semilogx(
            self.freqsISO,
            self.thresholdsISO,
            label='ISO'
        )
        plt.semilogx(
            self.freqsISO,
            self.predictions,
            color='r',
            linestyle='--',
            label='Predicted'
        )
        plt.legend()
        plt.show()


class DynamicLoudnessISOThresholdPredictor():
    def __init__(self,
                 model,
                 fs,
                 outputName=None,
                 loudnessFunction=None,
                 tol=0.1,
                 nIters=10,
                 alpha=1.0,
                 threshold=2.2):

        self.iterator = DynamicLoudnessIterator(model,
                                                fs,
                                                outputName,
                                                loudnessFunction,
                                                tol,
                                                nIters,
                                                alpha)
        # ISO data
        self.freqsISO = np.array([
            20.0, 25, 31.5, 40, 50, 63, 80, 100, 125, 160,
            200, 250, 315, 400, 500, 630, 750, 800, 1000, 1250, 1500, 1600,
            2000, 2500, 3000, 3150, 4000, 5000, 6000, 6300, 8000, 9000, 10000,
            11200, 12500, 14000
        ])

        self.thresholdsISO = np.array([
            78.5, 68.7, 59.5, 51.1, 44, 37.5, 31.5,
            26.5, 22.1, 17.9, 14.4, 11.4, 8.6, 6.2, 4.4, 3, 2.4, 2.2, 2.4, 3.5,
            2.4, 1.7, -1.3, -4.2, -5.8, -6.0, -5.4, -1.5, 4.3, 6, 12.6, 13.9,
            13.9, 13, 12.3, 18.4
        ])

        self.threshold = threshold
        self.predictions = None
        self.fs = fs
        self.duration = 1.0

    def process(self):

        self.predictions = np.zeros(self.freqsISO.size)
        for i, freq in enumerate(self.freqsISO):

            s = Sound.tone([freq], dur=self.duration, fs=self.fs)
            s.applyRamp(0.1)
            s.useDBSPL()
            s.normalise(self.thresholdsISO[i], 'RMS')

            self.predictions[i] = self.thresholdsISO[i]
            self.predictions[i] += self.iterator.process(s.data,
                                                         self.threshold)

    def plotPredictions(self):

        plt.semilogx(
            self.freqsISO,
            self.thresholdsISO,
            label='ISO'
        )
        plt.semilogx(
            self.freqsISO,
            self.predictions,
            color='r',
            linestyle='--',
            label='Predicted'
        )
        plt.legend()
        plt.show()
