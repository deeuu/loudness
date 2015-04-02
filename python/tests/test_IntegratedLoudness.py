import numpy as np
import matplotlib.pyplot as plt
import loudness as ln

#input signal bank
inputBank = ln.SignalBank()
nEars = 2
nChannels = 2
inputBank.initialize(nEars, nChannels, 1, 1)
inputBank.setFrameRate(1000)
inputBank.setChannelSpacingInCams(0.5)

#Loudness integration module
intLoudModule = ln.IntegratedLoudnessGM("GM2002", 1)
intLoudModule.initialize(inputBank)
intLoudBank = intLoudModule.getOutput()

#input output
nFrames = 3000
inputSpecificLoudness = np.repeat(np.ones(nFrames)*0.5, nEars *\
        nChannels).reshape((nEars,nChannels, 1, nFrames))
inputSpecificLoudness[:,:,:,(nFrames/4):] = 0
outputLoudness = np.zeros((nFrames, 3))

#processing
for i in range(nFrames):
    inputBank.setSignals(inputSpecificLoudness[:,:,:,i])
    intLoudModule.process(inputBank)
    outputLoudness[i] = intLoudBank.getSignals()[0, :, 0]

#data for plot
t = np.arange(1, nFrames+1) / 1000.0 #offet by 1 for annotation alignment
attackTimeSTL = np.round(intLoudModule.getAttackTimeSTL(),2)
releaseTimeSTL = np.round(intLoudModule.getReleaseTimeSTL(),2)
attackTimeLTL = np.round(intLoudModule.getAttackTimeLTL(),2)
releaseTimeLTL = np.round(intLoudModule.getReleaseTimeLTL(),2)
c2 = np.exp(-1)
c1 = 1-c2

#The plot
plt.figure(1)
plt.plot(t, outputLoudness)
plt.annotate(str(attackTimeSTL)+'s', (attackTimeSTL, c1))
plt.annotate(str(releaseTimeSTL)+'s', (0.75+releaseTimeSTL, c2))
plt.annotate(str(attackTimeLTL)+'s', (attackTimeLTL, c1 + 0.1))
plt.annotate(str(releaseTimeLTL)+'s', (0.75+releaseTimeLTL, c2 + 0.1))
plt.xlabel('Time, s')
plt.ylabel('Loudness, sones')
plt.show()
