import matplotlib.pylab as plt
import numpy as np
import loudness as ln

#Audio loader
fs = 32000
hopSize = 32
audio = ln.AudioFileCutter("../wavs/tone1kHz40dBSPL.wav", hopSize)
audio.initialize()
audioBank = audio.getOutput()
nFrames = audio.getNFrames()

#Create the loudness model
model = ln.DynamicLoudnessGM("../filterCoefs/32000_IIR_23_freemid.npy")
#model.setGoertzel(True)
model.initialize(audioBank)
loudnessBank = model.getModuleOutput(model.getNModules()-1)
nChannels = loudnessBank.getNChannels()

#storage
out = np.zeros((nFrames, nChannels))

#processing
for frame in range(nFrames):
    
    audio.process()
    model.process(audioBank)

    for chn in range(nChannels):
        out[frame, chn] = loudnessBank.getSample(chn,0)

#time points as centre of window
T = model.getTimeStep()
t = np.arange(1, nFrames+1)*T - 0.032
plt.figure(1)
plt.plot(t, out)
plt.xlabel("Time, s")
plt.ylabel("Loudness, sones")
plt.legend(("IL", "STL", "LTL"), 0)
plt.xlim((0, nFrames*T))
plt.show()
