import numpy as np
from scipy.signal import lfilter
import loudness as ln

nSources = 1
nEars = 2
nSamples = 10
nCoefficients = 128
blockSize = nSamples / 2

x = np.random.randn(2, nSamples)
b = np.random.randn(nCoefficients)

y = lfilter(b, [1.0], x)

bank = ln.SignalBank()
bank.initialize(nSources, nEars, 1, blockSize, 1)

fir = ln.FIR(b)
fir.initialize(bank)
outBank = fir.getOutput()

out = np.zeros(x.shape)

for i in range(2):

    start = blockSize*i
    end = start + blockSize

    for ear in range(nEars):
        print x[ear, start:end].shape
        bank.setSignal(0, ear, 0, x[ear, start:end])

    fir.process(bank)

    for ear in range(nEars):
        out[ear, start:end] = outBank.getSignal(0, ear, 0)

print np.allclose(out, y)
