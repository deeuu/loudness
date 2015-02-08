import loudness as ln
import numpy as np
from Sound import Sound
from LoudnessExtractor import LoudnessExtractor

model = ln.DynamicLoudnessGM()
model.setTimeStep(0.001)
extractor = LoudnessExtractor(model)
signal = Sound.tone(1000, dur = 0.2, fs = 32e3)
print signal.data.shape
signal.ref = 2e-5
signal.normalise(40)
extractor.process(signal.data[:,0])
extractor.plotLoudness()
