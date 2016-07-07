import numpy as np
import matplotlib.pyplot as plt
import loudness as ln

exp = 0.63
model = ln.DynamicLoudnessSimple()
model.setAlpha(exp)
model.setFilterSpacingInCams(0.25)
model.setCompressionCriterionInCams(0.3)

outputsOfInterest = [
    "ShortTermLoudness",
]

fs = 44100
extractor = ln.tools.extractors.DynamicLoudnessExtractor(
    model, fs, outputsOfInterest,
)

signal = ln.tools.sound.Sound.tone([100], dur=1.0, fs=fs)
signal.normalise(-23, "RMS")
signal.applyRamp(0.1)

extractor.process(signal.data)

times = extractor.outputDict['FrameTimes']
stl = extractor.outputDict['ShortTermLoudness']
print (1.0/exp) * 10 * np.log10(np.max(stl))
print (1.0/exp) * 10 * np.log10(np.mean(stl))
plt.plot(times, stl)
plt.xlabel('Time, s')
plt.ylabel('Loudness, sones')
plt.show()
