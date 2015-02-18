import loudness as ln
import numpy as np
from scipy.signal import butter, lfilter

b, a = butter(2,100.0/32e3,'highpass')

sigIn = ln.SignalBank()
sigIn.initialize(1,32,32000)
x = np.random.randn(32)
sigIn.setSignal(0, x)

filt = ln.Biquad(b, a)
filt.initialize(sigIn)
sigOut = filt.getOutput()

filt.process(sigIn)
y = lfilter(b, a, x)

print "Equality test: ", np.allclose(y, sigOut.getSignal(0))
