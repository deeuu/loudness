import numpy as np
import loudness as ln

windowSize = 20
xLen = 40
bufSize = 10
numFrames = xLen / bufSize

# Input and expected output (moving average)
x = np.random.randn(xLen)
y = np.convolve(x, np.ones(windowSize) / float(windowSize))[0:xLen]

# Configure an input bank with 2 ears & 2 channels
sig = ln.SignalBank()
sig.initialize(2, 2, bufSize, 1000)

# Simple moving average only
sma = ln.SMA(windowSize, True, False)
sma.initialize(sig)
out = sma.getOutput()

for i in range(numFrames):
    print "Frame", i
    start = i * bufSize
    end = start + bufSize
    sig.setSignals(
        np.tile(x[start:end], (sig.getNEars(), sig.getNChannels(), 1)))
    sma.process(sig)
    for ear in range(sig.getNEars()):
        for chn in range(sig.getNChannels()):
            print("Equality test for signal at Ear %d Chn %d : %r"
                  % (ear,
                     chn,
                     np.allclose(out.getSignal(ear, chn), y[start:end])))
