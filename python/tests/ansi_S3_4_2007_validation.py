import matplotlib.pylab as plt
import numpy as np
import loudness as ln
import sys,os
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)),'../'))
from sound import Sound
from loudnessExtractor import LoudnessExtractor

def loudnessOfPureTone(model, fs = 32e3, levels = 40, freq = 1000, dBSPL = True):
    '''
    Computes the loudness of a pure tone at a speified frequency and level. 
    If levels is a list, the loudness will be computed for each level.
    Model can be dynamic or steady-state.
    '''

    if type(levels) is not list:
        levels = [levels]

    #For dynamic models, we use LoudnessExtractor
    if(model.isDynamicModel()):
        extractor = LoudnessExtractor(model, fs, 1)
    else:
        #Configure input spectrum for single pure tone
        buf = ln.SignalBank()
        buf.initialize(1, 1, 1, fs)
        #initialise model
        model.initialize(buf)
        #final output should be integrated loudness
        outputBank = model.getModelOutput()

    #output loudness - could be multiple channels but definitely single sample
    #Global loudness -> one ear
    outputLoudness = np.zeros((levels.size, outputBank.getNChannels()))

    for i, level in enumerate(levels):
        if(model.isDynamicModel()):
            tone = Sound.tone(1000, dur = 2.5,fs=fs)
            if dBSPL:
                tone.ref = 2e-5
            tone.normalise(level, "RMS")
            tone.applyRamp(0.1) 
            extractor.process(tone.data[:,0])
            outputLoudness[i] =\
                extractor.computeGlobalLoudness(0.2,2.2,'MEAN')[0]
        else:
            buf.setSample(0, 0, 10**(level/10.0))
            model.process(buf)
            outputLoudness[i] = outputBank.getSample(0,0)
            #reset model state 
            model.reset()
        print "Input level (dB SPL):", level
        print "Loudness:", outputLoudness[i]
        
    return outputLoudness


if __name__ == '__main__':

    modelToEvaluate = "steadyLoudnessANSIS3407"
    fs = 32000

    if modelToEvaluate == "glasbergAndMoore2002":
        model = ln.DynamicLoudnessGM("../../filterCoefs/32000_FIR_4096_freemid.npy")
        model.loadParameterSet(0)
        model.setAnsiSpecificLoudness(True)
    elif modelToEvaluate == "steadyLoudnessANSIS3407":
        model = ln.SteadyLoudnessANSIS3407()

    #Table 7
    phons = np.array([0,1,2,3,4,5,7.5])
    phons = np.append(phons, np.arange(10, 125, 5))

    sones = np.array([0.0011, 0.0018, 0.0028, 0.0044, 0.0065, 0.0088, 0.017,
        0.029, 0.070, 0.142, 0.255, 0.422, 0.662, 0.997, 1.461, 2.098, 2.970,
        4.166, 5.813, 8.102, 11.326, 15.980, 22.929, 33.216, 48.242, 70.362,
        103.274, 152.776, 227.855, 341.982])

    loudnessToneTest(model, fs, phons, 1000);
