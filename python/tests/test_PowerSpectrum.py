import numpy as np
import loudness as ln

# Input setup
fs = 32000
frameSizes = np.array([2048, 1024, 512])
nEars = 2  # hard-coded for two ears
nBands = frameSizes.size
x = np.random.randn(nEars, nBands, frameSizes[0])
for frame in range(1, nBands):
    x[0, frame, frameSizes[frame]:] = 0
    x[1, frame, frameSizes[frame]:] = 0
x /= np.max(np.abs(x), 2).reshape((nEars, frameSizes.size, 1))

# Initialisation
inputBuf = ln.SignalBank()
inputBuf.initialize(nEars, nBands, frameSizes[0], fs)
inputBuf.setSignals(x)

# Power spectrum setup
bandFreqs = np.array([10, 500, 5000, 15001])
uniform = True
spectrumModule = ln.PowerSpectrum(bandFreqs, frameSizes, uniform)
spectrumModule.initialize(inputBuf)

spectrumBank = spectrumModule.getOutput()
spectrumLoudness = spectrumBank.getSignals()

# Processing
spectrumModule.process(inputBuf)

# numpy side
fftSizes = np.zeros(nBands, dtype='int')
for band in range(nBands):
    if uniform:
        fftSizes[band] = int(2 ** np.ceil(np.log2(frameSizes[0])))
    else:
        fftSizes[band] = int(2 ** np.ceil(np.log2(frameSizes[band])))

bandBinIndices = np.zeros((nBands, 2))
nBins = 0
for band in range(nBands):
    bandBinIndices[band][0] = np.ceil(
        bandFreqs[band] * fftSizes[band] / float(fs))
    bandBinIndices[band][1] = np.ceil(
        bandFreqs[band + 1] * fftSizes[band] / float(fs)) - 1
    nBins += bandBinIndices[band][1] - bandBinIndices[band][0] + 1

spectrumNumpy = np.zeros((nEars, nBins))
idxLoIn = 0
for band in range(nBands):
    X = np.fft.fft(x[:, band, 0:frameSizes[band]], fftSizes[band])
    idxLo = bandBinIndices[band][0]
    idxHi = bandBinIndices[band][1] + 1
    idxHiIn = idxLoIn + (idxHi - idxLo)
    spectrumNumpy[0, idxLoIn:idxHiIn] = 2 * np.abs(X[0, idxLo:idxHi]) ** 2 /\
        (frameSizes[band] * fftSizes[band])
    spectrumNumpy[1, idxLoIn:idxHiIn] = 2 * np.abs(X[1, idxLo:idxHi]) ** 2 /\
        (frameSizes[band] * fftSizes[band])
    idxLoIn = idxHiIn

# check
if np.allclose(spectrumLoudness[:, :, 0], spectrumNumpy):
    print "Numpy vs loudness power spectrum test: successful"
else:
    print "Numpy vs loudness power spectrum test: unsuccessful"
