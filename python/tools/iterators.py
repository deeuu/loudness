import numpy as np
import loudness as ln
from extractors import LoudnessExtractor, StationaryLoudnessExtractor

def asIs(x):
    return x

class GlobalDynamicLoudnessIterator():
    
    def __init__(self, 
            model, 
            fs,
            globalLoudnessFeature = None,
            loudnessLevelFunction = None,
            nInputEars = 1):

        self.extractor = LoudnessExtractor(model, fs, nInputEars)
        self.outputName = model.getOutputModulesToAggregate()[0]

        if globalLoudnessFeature is None:
            self.globalLoudnessFeature = np.mean
        else:
            self.globalLoudnessFeature = globalLoudnessFeature
        
        if loudnessLevelFunction is None:
            self.loudnessLevelFunction = asIs
        else:
            self.loudnessLevelFunction = loudnessLevelFunction

    def extractLoudness(self, signal, gainInDecibels = 0):
        signal *= 10 ** (gainInDecibels / 20.0)
        self.extractor.process(signal)
        timeSeries = self.extractor.outputDict[self.outputName]
        loudness = self.globalLoudnessFeature(timeSeries)
        loudnessLevel = self.loudnessLevelFunction(loudness)
        return loudnessLevel

    def process(self, inputSignal, targetLoudness, tol = 0.1, nIters = 5):

        # First check if the target loudness is a signal
        if type(targetLoudness) is np.ndarray:
            targetLoudness = self.extractLoudness(targetLoudness)

        gain = 0
        storedGain = 0

        for i in range(nIters):

            loudnessLevel = self.extractLoudness(inputSignal, gain)

            error = targetLoudness - loudnessLevel

            gain = error

            print (('Gain: %0.3f, Loudness Level: %0.3f, ' +
                    'Error: %0.3f') % (storedGain, loudnessLevel, error))

            if np.abs(error) < tol:
                break
            else:
                storedGain += gain

        return storedGain

class StationaryLoudnessIterator():

    def __init__(self, model, outputName, loudnessLevelFunction = None):
        
        self.outputName = outputName
        self.extractor = StationaryLoudnessExtractor(model, outputName)

        if loudnessLevelFunction is None:
            self.loudnessLevelFunction = asIs
        else:
            self.loudnessLevelFunction = loudnessLevelFunction

    def extractLoudness(self, frequencies, intensityLevels, gainInDecibels = 0):
        intensityLevels += gainInDecibels
        self.extractor.process(frequencies, intensityLevels)
        loudness = self.extractor.outputDict[self.outputName]
        loudnessLevel = self.loudnessLevelFunction(loudness)
        return loudnessLevel

    def process(self, frequencies, intensityLevels,
            targetLoudnessFrequencies, 
            targetLoudnessIntensityLevels, 
            tol, 
            nIters = 5):

        if type(targetLoudnessIntensityLevels) is np.ndarray:
            targetLoudness = self.extractLoudness(targetLoudnessFrequencies,
                    targetLoudnessIntensityLevels)
        else:
            targetLoudness = targetLoudnessIntensityLevels

        gain = 0
        storedGain = 0

        for i in range(nIters):

            loudnessLevel = self.extractLoudness(frequencies, intensityLevels, gain)

            error = targetLoudness - loudnessLevel

            gain = error

            print (('Gain: %0.3f, Loudness Level: %0.3f, ' +
                    'Error: %0.3f') % (storedGain, loudnessLevel, error))

            if np.abs(error) < tol:
                break
            else:
                storedGain += gain

        return storedGain
