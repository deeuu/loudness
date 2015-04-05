import matplotlib.pylab as plt
import numpy as np
import loudness as ln
from audiolab import wavread

#Audio loader
fs = 32000
frameSize = 32
fileToLoad = "../../wavs/pureTones/pureTone_1000Hz_40dBSPL_32000Hz.wav"
audio = ln.AudioFileCutter(fileToLoad, frameSize)
audio.initialize()
audioBank = audio.getOutput()
nFrames = audio.getNFrames()

out = np.zeros((audioBank.getNEars(), nFrames, frameSize))
for frame in range(nFrames):
    audio.process()
    for ear in range(audioBank.getNEars()):
        out[ear, frame] = audioBank.getSignal(ear,0)

x, fs, enc = wavread(fileToLoad)
y = out.flatten()[0:x.size]

if np.allclose(y, x):
    print "Test comparing result of AudioFileCutter with audiolab's wavread: successful"
else:
    print "Test comparing result of AudioFileCutter with audiolab's wavread: unsuccessful"
