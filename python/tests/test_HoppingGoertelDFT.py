import numpy as np
import loudness as ln
import matplotlib.pyplot as plt

def periodicHann(N):
    n = np.arange(0, N)
    window = 0.5 - 0.5 * np.cos(2*np.pi*n/N)
    return window

applyHannWindow = False
gain = 1.0
if applyHannWindow:
    gain = 0.25

fs = 48000
windowSizes = np.array([1024, 512])
nBands = windowSizes.size
hopSize = 256
blockSize = 128
bands = np.array([100, 5000, 10000])

windowStart = np.zeros(nBands)
bins = np.zeros((nBands, 2), dtype = 'int')
nBins = np.zeros(nBands, dtype = 'int')
hannWindows = []
for i in range(nBands):
    bins[i][0] = int(np.ceil(bands[i] * windowSizes[i] / float(fs)))
    bins[i][1] = int(np.ceil(bands[i+1] * windowSizes[i] / float(fs)))
    nBins[i] = bins[i][1] - bins[i][0]
    print bins[i] * fs / float(windowSizes[i])
    windowStart[i] = windowSizes[0]/2 - windowSizes[i]/2
    if applyHannWindow:
        hannWindows.append(periodicHann(windowSizes[i]))

nTotalBins = np.sum(nBins)
bankIn = ln.SignalBank()
bankIn.initialize(1, 1, blockSize, fs)
HGDFT = ln.HoppingGoertzelDFT(bands, windowSizes, hopSize, applyHannWindow)
HGDFT.initialize(bankIn)
bankOut = HGDFT.getOutput()

x = np.random.randn(windowSizes[0] * 48)

nFrames = int((x.size - windowSizes[0]) / float(hopSize)) + 1
outputFFTReal = np.zeros((nFrames, nTotalBins))
outputFFTImag = np.zeros((nFrames, nTotalBins))
outputHGDFTReal = np.zeros((nFrames, nTotalBins))
outputHGDFTImag = np.zeros((nFrames, nTotalBins))

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
        outputFFTReal[frame, binStart:binEnd] = np.real(X[bins[i][0]:bins[i][1]])
        outputFFTImag[frame, binStart:binEnd] = np.imag(X[bins[i][0]:bins[i][1]])
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
        outputHGDFTReal[frame, :] = flattenedSignal[::2] * gain
        outputHGDFTImag[frame, :] = flattenedSignal[1::2] * gain
        frame += 1

print outputFFTReal[0, 0]
print outputHGDFTReal[0, 0]
magFFT = np.sqrt(outputFFTReal.flatten()**2 + outputFFTImag.flatten()**2)
magHGDFT = np.sqrt(outputHGDFTReal.flatten()**2 + outputHGDFTImag.flatten()**2)
if np.allclose(magHGDFT, magFFT, 0.001, 0):
    print "Error in magnitude response is < 0.1 percent of FFT magnitude response"
else:
    print "Error in magnitude response is > 0.1 percent of FFT magnitude response"
# error function is periodic = peaks near DC and nyquist
e = magFFT - magHGDFT
plt.figure(1)
plt.plot(e)
plt.show()
