import numpy as np
import matplotlib.pyplot as plt
import loudness as ln

bank = ln.SignalBank()

bank.initialize(2, 1, 1, 1, 32000)
bank.setCentreFreq(0, 1000.0)
bank.setSample(0, 0, 0, 0, 10**(80/10.0))
bank.setSample(1, 0, 0, 0, 10**(80/10.0))

model = ln.StationaryLoudnessCHGM2011()
model.setOutputsToAggregate(['SpecificLoudness',
                             'SpecificPartialLoudness'])
model.initialize(bank)
model.process(bank)

specLoud = model.getOutput('SpecificLoudness')
specLoud = np.squeeze(specLoud.getSignals())

pSpecLoud = model.getOutput('SpecificPartialLoudness')
pSpecLoud = np.squeeze(pSpecLoud.getSignals())

#print 10 * np.log10(np.max(specLoud[0]) -  np.max(specLoud[1]) * 10 ** (-2/10.0))

plt.plot(specLoud[0], 'k')
plt.plot(specLoud[1])
plt.plot(pSpecLoud[0])
plt.show()
