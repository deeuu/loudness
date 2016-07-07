import numpy as np
import matplotlib.pyplot as plt
import loudness as ln

# centre frequencies to test
fc = np.array([500])

# input signal bank
inputBank = ln.SignalBank()
inputBank.initialize(2,  1, fc.size, 1, 1)
inputBank.setCentreFreqs(fc)

# Module for computing specific loudness
# True for ANSI S3.4 2007 mods
specLoudModule = ln.SpecificPartialLoudnessMGB1997(False)
specLoudModule2 = ln.SpecificPartialLoudnessMGB1997(True)
specLoudModule.initialize(inputBank)
specLoudModule2.initialize(inputBank)
specLoudBank = specLoudModule.getOutput()
specLoudBank2 = specLoudModule2.getOutput()

# input excitation levels
excDB = np.arange(0, 110, 0.01)
excLin = 10 ** (excDB / 10.0)

maskers = [-100, 10, 20, 30, 40, 50, 60, 70]
# output vector
specLoud = np.zeros((2, len(maskers), excLin.size))

for i, maskerLevel in enumerate(maskers):
    maskerIntensity = 10 ** (maskerLevel / 10.0)
    inputBank.setSample(1, 0, 0, 0, maskerIntensity) #Masker
    for j, lin in enumerate(excLin):
        inputBank.setSample(0, 0, 0, 0, lin) #Signal

        # process the excitation
        specLoudModule.process(inputBank)
        specLoudModule2.process(inputBank)

        # Output
        specLoud[0, i, j] = specLoudBank.getSample(0, 0, 0, 0)
        specLoud[1, i, j] = specLoudBank2.getSample(0, 0, 0, 0)


# ~2% error around 100~dB, possibly due to the parameter approximations
plt.figure(1)
plt.semilogy(excDB, specLoud[0].T, 'k')
plt.semilogy(excDB, specLoud[1].T, 'r--')
plt.xticks(np.arange(0, 120, 10))
plt.xlim(0, 110)
plt.ylim(0.004, 10)
plt.xlabel('Excitation, dB')
plt.ylabel('Specific Loudness, sones')
plt.tight_layout()
plt.show()
