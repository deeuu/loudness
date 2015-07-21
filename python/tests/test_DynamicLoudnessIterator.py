import sys
sys.path.append('../tools/')
import loudness as ln
import numpy as np

model = ln.DynamicLoudnessGM2002()
model.setRate(500)
model.setFilterSpacingInCams(0.75)

fs = 44100

def soneToPhon(x):
    return 40 + 10 * np.log2(x)

iterator = ln.tools.iterators.DynamicLoudnessIterator(model, fs, 'ShortTermLoudness', np.mean, soneToPhon)

signal = np.sin(2 * np.pi * 100.0 * np.arange(0, 1 * fs) / float(fs))

targetLoudness = signal * 0.5
iterator.process(signal, targetLoudness)
