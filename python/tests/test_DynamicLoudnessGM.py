import loudness as ln
import numpy as np
import sys,os
sys.path.append('../tools/')
from sound import Sound
from loudnessExtractor import LoudnessExtractor

model = ln.DynamicLoudnessGM('../../filterCoefs/32000_IIR_23_freemid.npy')
model.loadParameterSet("GM2002")
extractor = LoudnessExtractor(model, 32000, 1)

signal = Sound.tone(40, dur = 1.0, fs = 32e3)
signal.useDBSPL()
signal.normalise(40, "RMS")
signal.applyRamp(0.1)

extractor.process(signal.data)
extractor.plotLoudness()
