import loudness as ln

model = ln.DynamicLoudnessCH2012()
model.setRate(500)

outputsOfInterest = [
    "InstantaneousLoudness",
    "ShortTermLoudness",
    "LongTermLoudness",
    "PeakShortTermLoudness"
]

model.setOutputsToAggregate(outputsOfInterest)

fs = 44100
extractor = ln.tools.extractors.DynamicLoudnessExtractor(
    model, fs, 1, outputsOfInterest)
# align time 0 with centre of window
extractor.frameTimeOffset = -0.064 + extractor.timeStep

signal = ln.tools.sound.Sound.tone([1000], dur=1.0, fs=fs)
signal.useDBSPL()
signal.normalise(40, "RMS")
signal.applyRamp(0.1)

extractor.process(signal.data)
extractor.plotLoudnessTimeSeries(outputsOfInterest)
extractor.computeGlobalLoudnessFeatures(outputsOfInterest[:-1])
print extractor.outputDict['Features']['ShortTermLoudness']['Max']
