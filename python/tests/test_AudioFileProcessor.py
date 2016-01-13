import matplotlib.pyplot as plt
import loudness as ln

model = ln.DynamicLoudnessGM2002()
model.setRate(250)
model.setOutputModulesToAggregate(['LongTermLoudness'])

processor = ln.AudioFileProcessor(
    '../../wavs/pureTones/pureTone_1000Hz_40dBSPL_32000Hz.wav'
)
processor.setGainInDecibels(10)
processor.initialize(model)

processor.processAllFrames(model)

bankSTL = model.getOutputModuleSignalBank('LongTermLoudness')
aggregatedFrames = bankSTL.getAggregatedSignals()

plt.plot(aggregatedFrames.flatten())
plt.show()
