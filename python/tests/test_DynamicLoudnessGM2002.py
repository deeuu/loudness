import loudness as ln
import numpy as np
import sys,os
sys.path.append('../tools/')
from sound import Sound
from extractors import LoudnessExtractor

#model = ln.DynamicLoudnessGM2002('../../filterCoefs/32000_IIR_23_freemid.npy')
model = ln.DynamicLoudnessGM2002()
model.setPresentationDiotic(False)
model.setCompressionCriterionInCams(0.4)
model.setFilterSpacingInCams(0.25)
#model.setHoppingGoertzelDFTUsed(True)
#model.setSpectrumSampledUniformly(True)
model.setRate(500)
model.setPeakSTLFollowerUsed(False)

outputsOfInterest = ["InstantaneousLoudness", 
                     "ShortTermLoudness", 
                     "LongTermLoudness"]
model.setOutputModulesToAggregate(outputsOfInterest)

fs = 44100
extractor = LoudnessExtractor(model, fs, 2)
extractor.frameTimeOffset = -0.032 + extractor.timeStep #align time 0 with centre of window

signal = Sound.tone([1000, 1000], dur = 1.0, fs = fs)
signal.useDBSPL()
signal.normalise(40, "RMS")
signal.applyRamp(0.1)

extractor.process(signal.data)
extractor.plotLoudnessTimeSeries(outputsOfInterest)
