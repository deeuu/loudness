import numpy as np
import loudness as ln

#Input setup
fs = 32000
freq = 1000
freq2 = 6000
x = np.array([[np.sin(2*pi*freq*np.arange(fs)/fs)],
    [np.sin(2*pi*freq2*np.arange(fs)/fs)]])

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
x = x[:, :, 0:largestWindow]
nEars = 2
inputBuf = ln.SignalBank()
inputBuf.initialize(nEars, 1, largestWindow, fs)
inputBuf.setSignals(x)

window.setTargetModule(spectrum)
window.initialize(inputBuf)

#Processing
window.process(inputBuf)

#plot output frame
spectrumOut = spectrum.getOutput()
numBins = spectrumOut.getNChannels()
freqs = spectrumOut.getCentreFreqs()
frameSpectrum = spectrumOut.getSignals().reshape((2, numBins))
semilogx(freqs, 10*np.log10(frameSpectrum.T + 1e-10))
