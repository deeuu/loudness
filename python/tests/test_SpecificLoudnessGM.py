import numpy as np
import matplotlib.pyplot as plt
import loudness as ln

#centre frequencies to test
fc = np.array([52.0, 74, 108, 253, 500])

#input signal bank
inputBank = ln.SignalBank()
nEars = 2
inputBank.initialize(nEars, fc.size, 1, 1)
inputBank.setCentreFreqs(fc)

#Module for computing specific loudness
#True for ANSI S3.4 2007 mods
specLoudModule = ln.SpecificLoudnessGM(True)
specLoudModule.initialize(inputBank)
specLoudBank = specLoudModule.getOutput()

#input excitation levels
excDB = np.arange(0, 110, 0.01)
excLin = 10**(excDB/10.0)

#output vector
specLoud = np.zeros((nEars, fc.size, excLin.size))

for i, lin in enumerate(excLin):
    for chn in range(fc.size):
        for ear in range(nEars):
            #Set the excitation in each channel
            inputBank.setSample(ear, chn, 0, lin)

    #process the excitations
    specLoudModule.process(inputBank)

    #Output
    specLoud[:,:, i] = specLoudBank.getSignals()[:,:,0]


plt.figure(1)
plt.semilogy(excDB, specLoud[0].T, 'k')
plt.semilogy(excDB, specLoud[1].T, 'r--')
plt.xticks(np.arange(0, 120, 10))
plt.xlim(0,110)
plt.ylim(0.004,10)
plt.xlabel('Excitation, dB')
plt.ylabel('Specific Loudness, sones')
plt.tight_layout()
