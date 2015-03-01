import loudness as ln

input = ln.SignalBank()
input.initialize(1, 1024, 32000)
input.setSignal(0, np.ones(1024))
window = ln.Window("hann", [1024, 512], True, "rms")
window.setAlignOutput(False) #Use this for non-uniform spectral sampling

window.initialize(input)
windowOutput = window.getOutput()

spectrum = ln.PowerSpectrum([10,1000.0, 5000.0])

spectrum.initialize(windowOutput)
spectrumOut = spectrum.getOutput()

window.process(input)
spectrum.process(windowOutput)
