import numpy as np
import loudness as ln
import matplotlib.pyplot as plt


def periodicHann(N):
    n = np.arange(0, N)
    window = 0.5 - 0.5 * np.cos(2 * np.pi * n / N)
    return window

applyHannWindow = True

fs = 32000
x = np.random.randn(fs * 20)

windowSizes = [2048]
nBands = len(windowSizes)
hopSize = 256
blockSize = 256
bands = np.array([20, 5000.0])

windowStart = np.zeros(nBands)
bins = np.zeros((nBands, 2), dtype='int')
nBins = np.zeros(nBands, dtype='int')
hannWindows = []
normFactors = np.zeros(nBands)
for i in range(nBands):
    bins[i][0] = int(np.ceil(bands[i] * windowSizes[i] / float(fs)))
    print(bins[i][1])
    bins[i][1] = int(np.ceil(bands[i + 1] * windowSizes[i] / float(fs)))
    nBins[i] = bins[i][1] - bins[i][0]
    windowStart[i] = windowSizes[0] // 2 - windowSizes[i] // 2
    normFactors[i] = 2.0 / (2e-5 * 2e-5 * windowSizes[i] ** 2)
    if applyHannWindow:
        hannWindows.append(periodicHann(windowSizes[i]))
        normFactors[i] /= np.mean(hannWindows[i] ** 2)

nTotalBins = np.sum(nBins)
bankIn = ln.SignalBank()
bankIn.initialize(1, 1, 1, blockSize, fs)
HGDFT = ln.HoppingGoertzelDFT(
    bands, windowSizes, hopSize, applyHannWindow, True)
HGDFT.setFirstSampleAtWindowCentre(True)
HGDFT.initialize(bankIn)
bankOut = HGDFT.getOutput()

nFrames = int((x.size - windowSizes[0] // 2) / float(hopSize)) + 1

sFFT = np.zeros((nFrames, nTotalBins))
sHGDFT = np.zeros((nFrames, nTotalBins))

nBlocks = int(x.size / float(blockSize))
frame = 0
for block in range(nBlocks):
    start = block * blockSize
    end = start + blockSize
    bankIn.setSignal(0, 0, 0, x[start:end])
    HGDFT.process(bankIn)

    if bankOut.getTrig():
        flattenedSignal = bankOut.getSignals().flatten()
        sHGDFT[frame, :] = flattenedSignal
        frame += 1

# add half window zeros so window is centred at sample 0
x = np.hstack((np.zeros(windowSizes[0] // 2), x))
for frame in range(nFrames):
    binStart = 0
    for i in range(nBands):
        start = int(frame * hopSize + windowStart[i])
        end = int(start + windowSizes[i])
        if applyHannWindow:
            fftBuf = x[start:end] * hannWindows[i]
        else:
            fftBuf = x[start:end]
        X = np.fft.fft(fftBuf)
        binEnd = binStart + nBins[i]
        sFFT[frame, binStart:binEnd] = (normFactors[i] *
                                        np.abs(X[bins[i][0]:bins[i][1]]) ** 2)

        binStart += binEnd

sFFT = 10 * np.log10(sFFT.flatten())
sHGDFT = 10 * np.log10(sHGDFT.flatten())
if np.allclose(sHGDFT, sFFT, 0.01, 0):
    print ("Error in power spectra is < 0.01 % of FFT spectra")
else:
    print( "Error in power spectra is > 0.01 % of FFT spectra")
plt.figure(1)
plt.plot(sFFT)
plt.plot(sHGDFT)
plt.show()
e = sFFT - sHGDFT
plt.figure(2)
plt.plot(e)
plt.show()
