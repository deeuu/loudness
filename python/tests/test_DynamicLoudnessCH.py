import loudness as ln
import numpy as np
import sys,os
sys.path.append('../tools/')
from sound import Sound
from loudnessExtractor import LoudnessExtractor

model = ln.DynamicLoudnessCH()
model.setRate(500)
extractor = LoudnessExtractor(model, 32000, 1)

signal = Sound.tone(1000, dur = 1.0, fs = 32e3)
signal.ref = 2e-5
signal.normalise(40, "RMS")
signal.applyRamp(0.1)

extractor.process(signal.data)
extractor.plotLoudness()
