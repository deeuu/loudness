import numpy as np
import loudness as ln

fs = 44100

windowSizeSeconds = np.array([0.064, 0.032, 0.016, 0.008, 0.004, 0.002])
windowSize = np.round(windowSizeSeconds * fs).astype('int')
windowSize += windowSize%2 #Force even

window = ln.Window("hann", windowSize, True, True)

bank = ln.SignalBank()
bank.initialize(2, 1, windowSize[0], fs)
window.initialize(bank)

x = np.ones(windowSize[0])
for i, smp in enumerate(x):
    bank.setSample(0, 0, i, smp)
    bank.setSample(1, 0, i, smp)
window.process(bank)

bankOut = window.getOutput()
windows = np.zeros((bankOut.getNEars(), bankOut.getNChannels(), bankOut.getNSamples()))

for w in range(bankOut.getNChannels()):
    for i in range(bankOut.getNSamples()):
        windows[0, w, i] = bankOut.getSample(0, w, i)
        windows[1, w, i] = bankOut.getSample(1, w, i)
    plt.plot(windows[0,w], 'k')
    plt.plot(windows[1,w], 'r', linestyle='--')

