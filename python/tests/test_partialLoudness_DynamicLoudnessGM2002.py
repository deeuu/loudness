import numpy as np
import matplotlib.pyplot as plt
import loudness as ln

model = ln.DynamicLoudnessGM2002()
model.setRate(500)

outputsOfInterest = [
    "ShortTermLoudness",
    "ShortTermPartialLoudness",
]

fs = 44100
extractor = ln.tools.extractors.DynamicLoudnessExtractor(
    model, fs, outputsOfInterest, 4, 1
)

# align time 0 with centre of window
extractor.frameTimeOffset = -0.032 + extractor.timeStep

signal = ln.tools.sound.Sound.tone([1000], dur=1.0, fs=fs)
signal.useDBSPL()
signal.normalise(50, "RMS")
signal.applyRamp(0.1)

signal2 = ln.tools.sound.Sound.tone([500], dur=1.0, fs=fs)
signal2.useDBSPL()
signal2.normalise(80, "RMS")
signal2.applyRamp(0.1)

extractor.process([signal.data, signal2.data, signal.data, signal2.data])

times = extractor.outputDict['FrameTimes']
stl = extractor.outputDict['ShortTermLoudness']
stpl = extractor.outputDict['ShortTermPartialLoudness']
plt.plot(times, stl)
plt.plot(times, stpl)
plt.xlabel('Time, s')
plt.ylabel('Loudness, sones')
plt.show()
