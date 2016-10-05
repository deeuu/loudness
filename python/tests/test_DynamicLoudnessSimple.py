import soundAnalysis as sa
import numpy as np
import matplotlib.pyplot as plt
import loudness as ln

model = ln.DynamicLoudnessSimple()
model.setPresentationDiotic(False)

outputsOfInterest = [
    "ShortTermLoudness",
]

fs = 44100
extractor = ln.tools.extractors.DynamicLoudnessExtractor(
    model, fs, outputsOfInterest,
)

sound = sa.sounds.Sound.pureTone(1000, fs=fs, numChannels=1)
sound.applyRamp(0.05)

extractor.process(sound.data)

times = extractor.outputDict['FrameTimes']
stl = extractor.outputDict['ShortTermLoudness']
plt.plot(times, sa.signalTools.powerToDecibels(stl))
plt.xlabel('Time, s')
plt.ylabel('Loudness, dB')
plt.show()
