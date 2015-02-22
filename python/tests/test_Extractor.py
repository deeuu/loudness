import loudness as ln
import numpy as np
import sys,os
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)),'../'))
from Sound import Sound
from LoudnessExtractor import LoudnessExtractor

model = ln.DynamicLoudnessGM()
model.setTimeStep(0.001)
extractor = LoudnessExtractor(model)
signal = Sound.tone(1000, dur = 2.5, fs = 32e3)
signal.ref = 2e-5
signal.normalise(40)
signal.applyRamp(0.1)
extractor.process(signal.data[:,0])
print "Mean loudness: ", extractor.computeGlobalLoudness(0.6,2.0, 'MEAN')
print "Max loudness: ", extractor.computeGlobalLoudness(0.6,2.0, 'MAX')
print "N5 loudness: ", extractor.computeGlobalLoudness(0.6,2.0, 'N5')
print "Median loudness: ", extractor.computeGlobalLoudness(0.1,0.3, np.median)

extractor.plotLoudness()
