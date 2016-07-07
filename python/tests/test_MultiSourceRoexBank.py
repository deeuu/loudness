import matplotlib.pyplot as plt
import numpy as np
import loudness as ln

bank = ln.SignalBank()
fs = 32000
nSources = 2
nEars = 1
nChannels = 2
nSamples = 1
bank.initialize(nSources,
                nEars,
                nChannels,
                nSamples,
                fs)
bank.setCentreFreq(0, 100.0)
bank.setCentreFreq(1, 5000.0)

# Signal
levelOfSignal = 100
levelOfSignal2 = 40
bank.setSample(0, 0, 0, 0, 10 ** (levelOfSignal / 10.0))
bank.setSample(1, 0, 1, 0, 10 ** (levelOfSignal2 / 10.0))

roex = ln.MultiSourceRoexBank()
roex.initialize(bank)
roex.process(bank)

out = roex.getOutput()
ep = np.squeeze(out.getSignals())

plt.plot(10 * np.log10(ep.T + 1e-10))
plt.show()
