''' To do:
Add getters for normfactor and coefficients so can do equality test against
Table 1 of Hohmann (2002). fs = 16276.
Debug output looks good though.
'''
import matplotlib.pyplot as plt
import numpy as np
import loudness as ln


inputBank = ln.SignalBank()
fs = 16276
nfft = 2048
inputBank.initialize(1, 1, nfft, fs)
inputBank.setSample(0, 0, 0, 1.0)

camLow = ln.hertzToCam(73.2371)
camHigh = ln.hertzToCam(6681.39)
camStep = 1
gammatoneBank = ln.ComplexGammatoneBank(camLow,
                                        camHigh,
                                        camStep,
                                        4,
                                        False)

gammatoneBank.initialize(inputBank)
gammatoneBank.process(inputBank)

outputBank = gammatoneBank.getOutput()

outputSigs = np.squeeze(outputBank.getSignals())

halfNFFT = nfft/2 + 1
f = np.arange(halfNFFT) * fs / float(nfft)
for chn in range(outputSigs.shape[0]):
    X = np.fft.fft(outputSigs[chn], nfft)[:nfft/2 + 1]
    mx = np.abs(X)
    plt.plot(f, 20 * np.log10(mx + 1e-10), 'o-')
plt.show()
