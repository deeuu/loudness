import matplotlib.pyplot as plt
import loudness as ln

model = ln.DynamicLoudnessGM2002()
model.setRate(500)

outputsOfInterest = [
    "InstantaneousLoudness",
    "ShortTermLoudness",
    "LongTermLoudness"
]

fs = 44100
extractor = ln.tools.extractors.DynamicLoudnessExtractor(
    model, fs, outputsOfInterest,
)

# align time 0 with centre of window
extractor.frameTimeOffset = -0.032 + extractor.timeStep

signal = ln.tools.sound.Sound.tone([1000], dur=1.0, fs=fs)
signal.useDBSPL()
signal.normalise(40, "RMS")
signal.applyRamp(0.1)

extractor.process(signal.data)

times = extractor.outputDict['FrameTimes']
il = extractor.outputDict['InstantaneousLoudness']
stl = extractor.outputDict['ShortTermLoudness']
ltl = extractor.outputDict['LongTermLoudness']
plt.plot(times, il)
plt.plot(times, stl)
plt.plot(times, ltl)
plt.xlabel('Time, s')
plt.ylabel('Loudness, sones')
plt.show()
