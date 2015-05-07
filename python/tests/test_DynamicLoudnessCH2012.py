import loudness as ln
import numpy as np
import sys,os
sys.path.append('../tools/')
from sound import Sound
from extractors import LoudnessExtractor

model = ln.DynamicLoudnessCH2012()
model.setPresentationDiotic(False)
model.setRate(250)
model.setPeakSTLFollowerUsed(True)

outputsOfInterest = ["InstantaneousLoudness", 
                     "ShortTermLoudness", 
                     "LongTermLoudness", 
                     "PeakShortTermLoudness"]
extractor = LoudnessExtractor(model, 48e3, outputsOfInterest, 2)
extractor.frameTimeOffset = -0.064 + extractor.timeStep #align time 0 with centre of window

signal = Sound.tone([1000, 3000], dur = 1.0, fs = 48e3)
signal.useDBSPL()
signal.normalise(40, "RMS")
signal.applyRamp(0.1)

extractor.process(signal.data)
extractor.plotLoudnessTimeSeries(outputsOfInterest)
