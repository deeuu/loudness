import loudness as ln
import numpy as np
from extractors import StationaryLoudnessExtractor

model = ln.StationaryLoudnessANSIS342007()

freqs = np.array([3000.0])
intensityLevels = np.array([2.2])

extractor = StationaryLoudnessExtractor(model, 'InstantaneousLoudness')
extractor.process(freqs, intensityLevels)

print ln.soneToPhonMGB1997(float(extractor.outputDict['InstantaneousLoudness']), True)
