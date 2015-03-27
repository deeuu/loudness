import loudness as ln
import numpy as np
import sys,os
sys.path.append('../tools/')
from sound import Sound
from loudnessExtractor import LoudnessExtractor

model = ln.DynamicLoudnessGM()
model.setRate(500)
extractor = LoudnessExtractor(model, 32000, 1)

signal = Sound.tone(1000, dur = 2.5, fs = 32e3)
signal.useDBSPL()
signal.normalise(40, "RMS")
signal.applyRamp(0.1)

extractor.process(signal.data)

print "Mean loudness: ", extractor.computeGlobalLoudness(0.6,2.0, 'MEAN')
print "Max loudness: ", extractor.computeGlobalLoudness(0.6,2.0, 'MAX')
print "N5 loudness: ", extractor.computeGlobalLoudness(0.6,2.0, 'N5')
print "Median loudness: ", extractor.computeGlobalLoudness(0.1,0.3, np.median)

extractor.plotLoudness()
