import numpy as np
import loudness as ln
import matplotlib.pyplot as plt

def periodicHann(N):
    n = np.arange(0, N)
    window = 0.5 - 0.5 * np.cos(2*np.pi*n/N)
    return window

applyHannWindow = True

fs = 48000
windowSizes = np.array([1024, 512])
nBands = windowSizes.size
hopSize = 256
blockSize = 128
bands = np.array([10, 10000, 20000])

windowStart = np.zeros(nBands)
bins = np.zeros((nBands, 2), dtype = 'int')
nBins = np.zeros(nBands, dtype = 'int')
hannWindows = []
normFactors = np.zeros(nBands)
for i in range(nBands):
    bins[i][0] = int(np.ceil(bands[i] * windowSizes[i] / float(fs)))
    bins[i][1] = int(np.ceil(bands[i+1] * windowSizes[i] / float(fs)))
    nBins[i] = bins[i][1] - bins[i][0]
    windowStart[i] = windowSizes[0]/2 - windowSizes[i]/2
    normFactors[i] = 2.0 / (2e-5 * 2e-5 * windowSizes[i]**2)
    if applyHannWindow:
        hannWindows.append(periodicHann(windowSizes[i]))
        normFactors[i] *= np.mean(hannWindows[i]**2)

nTotalBins = np.sum(nBins)
bankIn = ln.SignalBank()
bankIn.initialize(1, 1, blockSize, fs)
HGDFT = ln.HoppingGoertzelDFT(bands, windowSizes, hopSize, applyHannWindow, True)
HGDFT.initialize(bankIn)
bankOut = HGDFT.getOutput()

x = np.random.randn(windowSizes[0] * 48)
nFrames = int((x.size - windowSizes[0]) / float(hopSize)) + 1

sFFT = np.zeros((nFrames, nTotalBins))
sHGDFT = np.zeros((nFrames, nTotalBins))

for frame in range(nFrames):
    binStart = 0
    for i in range(nBands):
        start = frame * hopSize + windowStart[i]
        end = start + windowSizes[i]
        fftBuf = x[start:end].copy()
        if applyHannWindow:
            fftBuf *= hannWindows[i]
        X = np.fft.fft(fftBuf)
        binEnd = binStart + nBins[i]
        sFFT[frame, binStart:binEnd] = (normFactors[i] *
                np.abs(X[bins[i][0]:bins[i][1]])**2)

        binStart += binEnd

nBlocks = int(x.size / float(blockSize))
frame = 0
for block in range(nBlocks):
    start = block * blockSize
    end = start + blockSize
    bankIn.setSignal(0, 0, x[start:end])
    HGDFT.process(bankIn)

    if bankOut.getTrig():
        flattenedSignal = bankOut.getSignals().flatten()
        sHGDFT[frame, :] = flattenedSignal
        frame += 1

sFFT = sFFT.flatten()
sHGDFT = sHGDFT.flatten()
if np.allclose(sHGDFT, sHGDFT, 0.001, 0):
    print "Error in power spectra is < 0.01 percent of FFT spectra"
else:
    print "Error in power spectra is > 0.01 percent of FFT spectra"
e  = sFFT - sHGDFT
plt.figure(1)
plt.plot(e)
plt.show()
