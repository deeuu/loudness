import loudness as ln
import numpy as np
import sys,os
sys.path.append('../tools/')
from sound import Sound
from loudnessExtractor import LoudnessExtractor

model = ln.DynamicLoudnessGM('../../filterCoefs/32000_IIR_23_freemid.npy')

outputsOfInterest = ["InstantaneousLoudness", "ShortTermLoudness", "LongTermLoudness"]
extractor = LoudnessExtractor(model, 32000, 1, outputsOfInterest)

signal = Sound.tone(1000, dur = 1.0, fs = 32e3)
signal.useDBSPL()
signal.normalise(40, "RMS")
signal.applyRamp(0.1)

extractor.process(signal.data)
extractor.plotLoudnessTimeSeries(outputsOfInterest)
