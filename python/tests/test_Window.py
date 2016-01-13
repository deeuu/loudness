import numpy as np
import matplotlib.pyplot as plt
import loudness as ln


def hann(N=1024, periodic=True, fullSize=2048):
    n = np.arange(0, N)
    if periodic:
        # Harris (1978) Eq 27b
        window = 0.5 - 0.5 * np.cos(2 * np.pi * n / N)
    else:
        # Produce zeros on both sides
        window = 0.5 - 0.5 * np.cos(2 * np.pi * n / (N - 1))
    if N < fullSize:
        window = np.hstack((window, np.zeros(fullSize - N)))
    return window

# Specification
fs = 32000
windowSizeSeconds = np.array([0.064, 0.032, 0.016, 0.008, 0.004, 0.002])
windowSize = np.round(windowSizeSeconds * fs).astype('int')
windowSize += windowSize % 2  # Force even

'''Loudness side'''
window = ln.Window(0, windowSize, True)
window.setNormalisation(0)
bank = ln.SignalBank()
# two ears
bank.initialize(2, 1, windowSize[0], fs)
window.initialize(bank)

# vector of ones
bank.setSignals(np.ones((2, 1, windowSize[0])))

# process
window.process(bank)

# output
bankOut = window.getOutput()
windowsLn = bankOut.getSignals()

# plot all windows in both ears
for w in range(bankOut.getNChannels()):
    plt.plot(windowsLn[0, w], 'k')
    plt.plot(windowsLn[1, w], 'r', linestyle='--')
plt.show()

'''Python side'''
windowsPy = [hann(N, True, windowSize[0]) for N in windowSize]

# Test windows are ~= in both ears
for w in range(windowSize.size):
    test = np.allclose(windowsLn[0, w], windowsPy[w]) * \
        np.allclose(windowsLn[1, w], windowsPy[w])
    print ("Equality test for window %d (both ears) : %r" % (w, test))
