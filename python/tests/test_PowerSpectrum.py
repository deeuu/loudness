import numpy as np
import loudness as ln

#Input setup
fs = 32000
freq = 1000
x = np.sin(2*pi*freq*np.arange(fs)/fs)
x += np.sin(2*pi*freq*2*np.arange(fs)/fs)
inputBuf = ln.SignalBank()

#Window setup
windowSizeSeconds = np.array([0.064, 0.032, 0.016, 0.008, 0.004, 0.002])
windowSize = np.round(windowSizeSeconds * fs).astype('int')
windowSize += windowSize%2
window = ln.Window("hann", windowSize, True)

#Power spectrum setup
bandSpec = np.array([10, 80, 500, 1250, 2540, 4050, 15001])
spectrum = ln.PowerSpectrum(bandSpec, True)

#Initialisation
largestWindow = windowSize[0]
x = x[0:largestWindow]
inputBuf.initialize(1, largestWindow, fs)
inputBuf.setSignal(0, x)
window.initialize(inputBuf)
windowOutput = window.getOutput()
spectrum.initialize(windowOutput)
spectrumOut = spectrum.getOutput()

#Processing
window.process(inputBuf)
spectrum.process(windowOutput)

numBins = spectrumOut.getNChannels()
freqs = spectrumOut.getCentreFreqs()
frameSpectrum = np.array([spectrumOut.getSample(i,0) for i in range(numBins)])

semilogx(freqs, 10*np.log10(frameSpectrum+1e-10))
