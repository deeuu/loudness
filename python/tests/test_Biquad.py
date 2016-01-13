import loudness as ln
import numpy as np
from scipy.signal import butter, lfilter


def updateFilterCoefficients(b, a, fsOld, fsNew):

    fc = (fsOld / np.pi) * \
        np.arctan(np.sqrt((1 + a[1] + a[2]) / (1 - a[1] + a[2])))
    Q = np.sqrt((a[2] + 1) ** 2 - a[1] ** 2) / (2 * np.abs(1 - a[2]))
    Vl = (b[0] + b[1] + b[2]) / (1 + a[1] + a[2])
    Vb = (b[0] - b[2]) / (1 - a[2])
    Vh = (b[0] - b[1] + b[2]) / (1 - a[1] + a[2])

    omega = np.tan(np.pi * fc / fsNew)
    omegaSqrd = omega * omega
    denom = omegaSqrd + omega / Q + 1

    b[0] = (Vl * omegaSqrd + Vb * (omega / Q) + Vh) / denom
    b[1] = 2 * (Vl * omegaSqrd - Vh) / denom
    b[2] = (Vl * omegaSqrd - (Vb * omega / Q) + Vh) / denom

    a[0] = 1.0
    a[1] = 2 * (omegaSqrd - 1) / denom
    a[2] = (omegaSqrd - (omega / Q) + 1) / denom

# design a high pass flter at 48kHz
b_48k, a_48k = butter(2, 100.0 / 48e3, 'highpass')
b_32k = b_48k.copy()
a_32k = a_48k.copy()

# solve for coeffcients at new fs = 32kHz
updateFilterCoefficients(b_32k, a_32k, 48e3, 32e3)

# Loudness side - input fs = 32kHz
sigIn = ln.SignalBank()
sigIn.initialize(1, 1, 2048, 32000)
x = np.random.randn(2048)
sigIn.setSignal(0, 0, x)

# input coefficients designed for 48kHz
filt = ln.Biquad(b_48k, a_48k)
filt.setCoefficientFs(48e3)
filt.initialize(sigIn)
sigOut = filt.getOutput()

# process using scipy's lfilter
yScipy = lfilter(b_32k, a_32k, x)

# process loudness side
filt.process(sigIn)
yLoudness = sigOut.getSignal(0, 0)

# equality test
if np.allclose(yLoudness, yScipy):
    print "Test comparing result of Biquad with python side filter" + \
        "estimation and scipy's lfilter: successful"
else:
    print "Test comparing result of Biquad with python side filter " + \
        "estimation and scipy's lfilter: unsuccessful"
