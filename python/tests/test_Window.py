import numpy as np
import loudness as ln

fs = 32000

windowSizeSeconds = np.array([0.064, 0.032, 0.016, 0.008, 0.004, 0.002])
windowSize = np.round(windowSizeSeconds * fs).astype('int')
windowSize += windowSize%2 #Force even

window = ln.Window("hann", windowSize, True)

bank = ln.SignalBank()
bank.initialize(2, 1, windowSize[0], fs)
window.initialize(bank)

bank.setSignals(np.ones((2,1,windowSize[0])))

window.process(bank)

bankOut = window.getOutput()

windows = bankOut.getSignals()

for w in range(bankOut.getNChannels()):
    plt.plot(windows[0,w], 'k')
    plt.plot(windows[1,w], 'r', linestyle='--')
