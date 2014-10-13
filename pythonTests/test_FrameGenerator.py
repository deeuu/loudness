import numpy as np
import loudness as ln

fs = 32000
N = 2048
x = np.arange(0, N)

#Input SignalBank
bufSize = 256
inputBank = ln.SignalBank()
inputBank.initialize(1, bufSize, int(fs))

#Frame generator
frameSize = 1024
hopSize = 512
frameGen = ln.FrameGenerator(frameSize, hopSize)
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
for i, frame in enumerate(frames):
    start = i*hopSize
    if not all(frame == np.arange(start, start+frameSize)):
        print("Frame number %d incorrect" % i)


