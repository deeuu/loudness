import matplotlib.pyplot as plt
import loudness as ln

model = ln.DynamicLoudnessGM2002()
model.setRate(250)
outputs = ['ShortTermLoudness', 'LongTermLoudness']
model.setOutputsToAggregate(outputs)

processor = ln.AudioFileProcessor('../../wavs/pureTones/pureTone_1000Hz_40dBSPL_32000Hz.wav')
processor.setGainInDecibels(10)
processor.initialize(model)

processor.processAllFrames(model)

for output in outputs:
    bankSTL = model.getOutput(output)
    aggregatedFrames = bankSTL.getAggregatedSignals()
    plt.plot(aggregatedFrames.flatten())
plt.show()
