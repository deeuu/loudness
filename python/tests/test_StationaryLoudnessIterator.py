import loudness as ln
import numpy as np
import sys
sys.path.append('../tools/')
from iterators import StationaryLoudnessIterator

def func(x):
    return ln.soneToPhonMGB1997(float(x), True)

freqs = np.array([1000.0])
levels = np.array([50.0])

freqs = np.array([3000.0])
target = np.array([60])

model = ln.StationaryLoudnessANSIS342007()
iterator = StationaryLoudnessIterator(model, 'InstantaneousLoudness', func)
iterator.process(freqs, levels, freqs, target, 0.1, 5)
