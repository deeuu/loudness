import loudness as ln
import numpy as np

model = ln.DynamicLoudnessCH2012()
model.setPresentationDiotic(False)
model.setRate(250)
model.setPeakSTLFollowerUsed(True)

outputsOfInterest = ["InstantaneousLoudness", 
                     "ShortTermLoudness", 
                     "LongTermLoudness", 
                     "PeakShortTermLoudness"]
model.setOutputModulesToAggregate(outputsOfInterest)

fs = 32000
extractor = ln.tools.extractors.LoudnessExtractor(model, fs, 2)
extractor.frameTimeOffset = -0.064 + extractor.timeStep #align time 0 with centre of window

signal = ln.tools.sound.Sound.tone([1000, 3000], dur = 1.0, fs = fs)
signal.useDBSPL()
signal.normalise(40, "RMS")
signal.applyRamp(0.1)

extractor.process(signal.data)
extractor.plotLoudnessTimeSeries(outputsOfInterest)
