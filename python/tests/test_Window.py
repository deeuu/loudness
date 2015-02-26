import loudness as ln

w = ln.Window("hann", [10, 6], True)
bank = ln.SignalBank()
bank.initialize(1,10,32000)
w.initialize(bank)

bank.setSignal(0, np.ones(10))
w.process(bank)

bankOut = w.getOutput()
windows = np.zeros((10,2))
windows[:,0] = bankOut.getSignal(0)
windows[:,1] = bankOut.getSignal(1)

plt.plot(windows)
