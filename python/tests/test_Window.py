import numpy as np
import loudness as ln

fs = 44100

windowSizeSeconds = np.array([0.064, 0.032, 0.016, 0.008, 0.004, 0.002])
windowSize = np.round(windowSizeSeconds * fs).astype('int')
windowSize += windowSize%2 #Force even

window = ln.Window("hann", windowSize, True)
window.setAlignOutput(True)

bank = ln.SignalBank()
bank.initialize(1, windowSize[0], fs)
window.initialize(bank)

bank.setSignal(0, np.ones(windowSize[0]))
window.process(bank)

bankOut = window.getOutput()
windows = np.zeros((windowSize[0], 6))
windows[:,0] = bankOut.getSignal(0)
windows[:,1] = bankOut.getSignal(1)
windows[:,2] = bankOut.getSignal(2)
windows[:,3] = bankOut.getSignal(3)
windows[:,4] = bankOut.getSignal(4)
windows[:,5] = bankOut.getSignal(5)

plt.plot(windows)
