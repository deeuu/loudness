import numpy as np
from scipy.signal import lfilter
import loudness as ln

nSamples = 1024
nCoefficients = 128
blockSize = nSamples / 2

x = np.random.randn(2, nSamples)
b, a = np.load('../../filterCoefs/44100_IIR_23_freemid.npy')
y = lfilter(b, a, x)

bank = ln.SignalBank()
bank.initialize(2, 1, blockSize, 44100)

iir = ln.IIR(b, a)
iir.initialize(bank)
outBank = iir.getOutput()

out = np.zeros(x.shape)

for i in range(2):

    start = blockSize*i
    end = start + blockSize

    for ear in range(2):
        bank.setSignal(ear, 0, x[ear, start:end])

    iir.process(bank)

    for ear in range(2):
        out[ear, start:end] = outBank.getSignal(ear, 0)

print np.allclose(out, y)
