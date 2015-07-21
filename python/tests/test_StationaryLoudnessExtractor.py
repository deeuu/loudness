import loudness as ln
import numpy as np

#model = ln.StationaryLoudnessANSIS342007()
model = ln.StationaryLoudnessCHGM2011()

freqs = np.array([1000.0])
intensityLevels = np.array([40])

extractor = ln.tools.extractors.StationaryLoudnessExtractor(model, 'InstantaneousLoudness')
extractor.process(freqs, intensityLevels)

print ln.soneToPhonMGB1997(float(extractor.outputDict['InstantaneousLoudness']), True)
