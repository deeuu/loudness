import numpy as np
import loudness as ln

windowSize = 5
xLen = 30
bufSize = 10
numFrames = xLen/bufSize

x = np.random.randn(xLen)
y = np.convolve(x, np.ones(windowSize)/float(windowSize))[0:xLen]
yTest = np.zeros(x.size)

sig = ln.SignalBank()
sig.initialize(1, bufSize, 1000)

sma = ln.SMA(windowSize, True, False)
sma.initialize(sig)
out = sma.getOutput()

for i in range(numFrames):
    start = i*bufSize
    end = start + bufSize
    sig.setSignal(0, x[start:end]) 
    sma.process(sig)
    yTest[start:end] = out.getSignal(0)

print "Equality test = ", np.allclose(y, yTest)
