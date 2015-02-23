import loudness as ln
import numpy as np
from scipy.signal import butter, lfilter

def updateFilterCoefficients(b, a, fsOld, fsNew):

    fc = (fsOld/np.pi) * np.arctan( np.sqrt( (1+a[1]+a[2])/(1-a[1]+a[2])))
    Q = np.sqrt((a[2]+1)**2 - a[1]**2)/(2*np.abs(1-a[2]))
    Vl = (b[0]+b[1]+b[2]) / (1+a[1]+a[2])
    Vb = (b[0] - b[2]) / (1-a[2])
    Vh = (b[0]-b[1]+b[2]) / (1-a[1]+a[2])

    omega = np.tan(np.pi*fc/fsNew)
    omegaSqrd = omega*omega
    denom = omegaSqrd + omega/Q + 1

    b[0] = (Vl * omegaSqrd + Vb * (omega/Q) + Vh) / denom
    b[1] = 2*(Vl*omegaSqrd - Vh) / denom
    b[2] = (Vl*omegaSqrd - (Vb*omega/Q) + Vh) / denom

    a[0] = 1.0
    a[1] = 2*(omegaSqrd - 1)/ denom
    a[2] = (omegaSqrd - (omega/Q) + 1)/ denom

b_48k, a_48k = butter(2,100.0/48e3,'highpass')
b_32k = b_48k.copy()
a_32k = a_48k.copy()

updateFilterCoefficients(b_32k, a_32k, 48e3, 32e3)

sigIn = ln.SignalBank()
sigIn.initialize(1,32,32000)
x = np.random.randn(32)
sigIn.setSignal(0, x)

filt = ln.Biquad(b_48k, a_48k)
filt.setCoefficientFs(48e3)
filt.initialize(sigIn)
sigOut = filt.getOutput()

filt.process(sigIn)
y = lfilter(b_32k, a_32k, x)

print "Equality test: ", np.allclose(y, sigOut.getSignal(0))
