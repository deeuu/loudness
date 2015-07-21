import loudness as ln
import numpy as np

model = ln.StationaryLoudnessANSIS342007()

freqs = np.array([1000.0])
intensityLevels = np.array([2.2])

extractor = ln.tools.extractors.StationaryLoudnessExtractor(model, 'InstantaneousLoudness')
extractor.process(freqs, intensityLevels)

sones = float(extractor.outputDict['InstantaneousLoudness'])
phons = ln.soneToPhonMGB1997(sones, True)
print 'Loudness in sones at threshold : %0.1g' % sones
print 'Loudness in phons at threshold : %0.2g' % phons
