import loudness as ln

hopSize = 32
audio = ln.AudioFileCutter('../../wavs/tone1kHz40dBSPL.wav', hopSize)
audio.initialize()
audioBank = audio.getOutput()

fs = 32000
audioBank = ln.SignalBank()
audioBank.initialize(1, 1, hopSize, 32000)

model = ln.DynamicLoudnessGM()
model.setHpf(False)
model.initialize(audioBank)
specOut = model.getModuleOutput("PowerSpectrum")

x = np.sin( 2 * pi * 1000 * np.arange(fs) / fs)
x *= 10**(-53 / 20.0)
for i in range(64):
    #audio.process()
    audioBank.setSignal(0, 0, x[i*hopSize:i*hopSize + hopSize])
    model.process(audioBank)

X = specOut.getSignals()
plot( 10 * log10( X[0,:,0] + 1e-10) )

windowBank = model.getModuleOutput("Window")

ps = ln.PowerSpectrum([10, 80, 500, 1250, 2540, 4050, 15001], [2048, 1024, 512,
    256, 128, 64], True)
ps.initialize(windowBank)
ps.process(windowBank)

psOut = ps.getOutput()
X2 = psOut.getSignals()

