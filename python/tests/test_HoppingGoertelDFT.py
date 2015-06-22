import numpy as np
import loudness as ln
import matplotlib.pyplot as plt

fs = 48000

windowSize = 1024
hopSize = 256
blockSize = 256
loFreq = 0.0
hiFreq = fs / 2.0

binLo = int(np.ceil(loFreq * windowSize / float(fs)))
binHi = int(np.ceil(hiFreq * windowSize / float(fs)))
nBins = binHi - binLo

bankIn = ln.SignalBank()
bankIn.initialize(1, 1, blockSize, fs)
HGDFT = ln.HoppingGoertzelDFT(loFreq, hiFreq, windowSize, hopSize)
HGDFT.initialize(bankIn)
bankOut = HGDFT.getOutput()

x = np.random.randn(windowSize * 48)

nFrames = int((x.size - windowSize) / float(hopSize)) + 1
outputFFTReal = np.zeros((nFrames, nBins))
outputFFTImag = np.zeros((nFrames, nBins))
outputHGDFTReal = np.zeros((nFrames, nBins))
outputHGDFTImag = np.zeros((nFrames, nBins))

for frame in range(nFrames):
    start = frame * hopSize
    end = start + windowSize
    fftBuf = x[start:end]
    X = np.fft.fft(fftBuf)
    outputFFTReal[frame, :] = np.real(X[binLo:binHi])
    outputFFTImag[frame, :] = np.imag(X[binLo:binHi])

nBlocks = int(x.size / float(blockSize))
frame = 0
for block in range(nBlocks):
    start = block * blockSize
    end = start + blockSize
    bankIn.setSignal(0, 0, x[start:end])
    HGDFT.process(bankIn)

    if bankOut.getTrig():
        flattenedSignal = bankOut.getSignals().flatten()
        outputHGDFTReal[frame, :] = flattenedSignal[::2]
        outputHGDFTImag[frame, :] = flattenedSignal[1::2]
        frame += 1

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
