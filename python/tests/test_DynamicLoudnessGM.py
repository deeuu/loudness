import matplotlib.pylab as plt
import numpy as np
import loudness as ln
import sys 
sys.path.append('../tools/')
from sound import Sound

#Audio loader
hopSize = 32
audio = ln.AudioFileCutter("../../wavs/pureTones/pureTone_1000Hz_40dBSPL_32000Hz.wav", hopSize)
audio.initialize()
audioBank = audio.getOutput()
nFrames = audio.getNFrames()

#Create the loudness model
model = ln.DynamicLoudnessGM("../../filterCoefs/32000_IIR_23_freemid.npy")
model = ln.DynamicLoudnessGM()
model.initialize(audioBank)
loudnessBank = model.getModuleOutput("IntegratedLoudnessGM")
nChannels = loudnessBank.getNChannels()

#storage
out = np.zeros((nFrames, nChannels))

#processing
frame = 0
i = 0
while frame < nFrames:
    audio.process()
    i += 1
    model.process(audioBank)

    if loudnessBank.getTrig():
        out[frame] = loudnessBank.getSignals()[0,:,0]
        frame += 1

#time points as centre of analysis window
T = model.getTimeStep()
t = np.arange(0, nFrames)*T
plt.figure(1)
plt.plot(t, out)
plt.xlabel("Time, s")
plt.ylabel("Loudness, sones")
plt.legend(("IL", "STL", "LTL"), 0)
plt.xlim((0, nFrames*T))
plt.show()
