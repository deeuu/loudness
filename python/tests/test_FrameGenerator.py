import numpy as np
import loudness as ln

fs = 32000
N = 10000
x = np.arange(0, N)

#Input SignalBank
bufSize = 32
nEars = 2
nChannels = 1
inputBank = ln.SignalBank()
inputBank.initialize(nEars, nChannels, bufSize, int(fs))

#Frame generator
frameSize = 2048
hopSize = 32
startAtZero = False
frameGen = ln.FrameGenerator(frameSize, hopSize, startAtZero)
frameGen.initialize(inputBank)
outputBank = frameGen.getOutput()

nBlocks = int(x.size / bufSize)
if startAtZero:
    nProcessedBlocks = int(nBlocks - frameSize/hopSize + 1)
else:
    nProcessedBlocks = int(nBlocks - 0.5*frameSize/hopSize + 1)
frames = np.zeros((nEars, nProcessedBlocks, frameSize))

frameIndex = 0
for block in range(nBlocks):
    #Update input bank
    idx = block*bufSize
    for ear in range(nEars):
        [inputBank.setSample(ear,0,i, smp) for i, smp in enumerate(x[idx:idx + bufSize])]

    #process it
    frameGen.process(inputBank)

    #get output
    if(outputBank.getTrig()):
        for ear in range(nEars):
            frames[ear, frameIndex, :] = [outputBank.getSample(ear,0,i) for i in range(frameSize)]
        frameIndex += 1

#Check frames are correct
if not startAtZero:
    x = np.hstack((np.zeros(np.ceil((frameSize-1)/2.0)), x))

for ear in range(nEars):
    for i, frame in enumerate(frames[ear]):
        start = i*hopSize
        if all(frame == x[start:start+frameSize]):
            print("Frame number %d correct" % i)
        else:
            print("Frame number %d incorrect" % i)
