import numpy as np
import loudness as ln

fs = 32000
N = 10000
x = np.arange(0, N)

#Input SignalBank
bufSize = 32
inputBank = ln.SignalBank()
inputBank.initialize(1, bufSize, int(fs))

#Frame generator
frameSize = 2048
hopSize = 32
startAtZero = False
frameGen = ln.FrameGenerator(frameSize, hopSize, startAtZero)
frameGen.initialize(inputBank)
outputBank = frameGen.getOutput()

frames = []
for block in range(x.size/bufSize):
    #Update input bank
    idx = block*bufSize
    inputBank.setSignal(0, x[idx:idx + bufSize])

    #process it
    frameGen.process(inputBank)

    #get output
    if(outputBank.getTrig()):
        frames.append(outputBank.getSignal(0))

#Check frames are correct
frames = np.array(frames)
if not startAtZero:
    x = np.hstack((np.zeros(np.ceil((frameSize-1)/2.0)), x))
for i, frame in enumerate(frames):
    start = i*hopSize
    if all(frame == x[start:start+frameSize]):
        print("Frame number %d correct" % i)
    else:
        print("Frame number %d incorrect" % i)
