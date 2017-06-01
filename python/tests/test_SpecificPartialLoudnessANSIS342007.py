import numpy as np
import matplotlib.pyplot as plt
import loudness as ln

'''
For bands with low excitaiton levels the specific partial loudness seems to be
> than the specific loudness.  

This is because the excitation pattern used to calculated the specific partial
loudness is not identical to the one used to compute the specific loudness. If
noise is present, then the auditory filters are broader (generally speaking)
than those used for signal in quiet (see Moore et al., 1997 for more details).
'''

bank = ln.SignalBank()

bank.initialize(2, 1, 1, 1, 32000)
bank.setCentreFreq(0, 5000.0)
bank.setSample(0, 0, 0, 0, 10**(58/10.0))
bank.setSample(1, 0, 0, 0, 10**(60/10.0))

model = ln.StationaryLoudnessANSIS342007()
model.setOutputsToAggregate(['Excitation',
                             'SpecificLoudness',
                             'SpecificPartialLoudness'])
model.initialize(bank)
model.process(bank)

ep = model.getOutput('Excitation')
ep = np.squeeze(ep.getSignals())

specLoud = model.getOutput('SpecificLoudness')
specLoud = np.squeeze(specLoud.getSignals())

pSpecLoud = model.getOutput('SpecificPartialLoudness')
pSpecLoud = np.squeeze(pSpecLoud.getSignals())

#print 10 * np.log10(np.max(specLoud[0]) -  np.max(specLoud[1]) * 10 ** (-2/10.0))

plt.plot(10 * np.log10(ep[0] + 1e-10))
plt.plot(10 * np.log10(ep[1] + 1e-10))
plt.show()

plt.plot((specLoud[1] + 1e-10))
plt.plot((specLoud[0] + 1e-10), 'k')
plt.plot((pSpecLoud[0] + 1e-10), 'k--')
plt.show()
