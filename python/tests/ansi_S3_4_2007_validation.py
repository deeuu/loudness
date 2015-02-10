import matplotlib.pylab as plt
import numpy as np
import loudness as ln
import sys,os
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)),'../'))
from Sound import Sound
from LoudnessExtractor import LoudnessExtractor

def loudness1kHz(model, fs=32e3):

    #Table 7
    phons = np.array([0,1,2,3,4,5,7.5])
    phons = np.append(phons, np.arange(10, 125, 5))

    sones = np.array([0.0011, 0.0018, 0.0028, 0.0044, 0.0065, 0.0088, 0.017,
        0.029, 0.070, 0.142, 0.255, 0.422, 0.662, 0.997, 1.461, 2.098, 2.970,
        4.166, 5.813, 8.102, 11.326, 15.980, 22.929, 33.216, 48.242, 70.362,
        103.274, 152.776, 227.855, 341.982])

    if(model.isDynamicModel()):
        extractor = LoudnessExtractor(model)
    else:
        #Configure input spectrum for single 1kHz pure tone
        buf = ln.SignalBank()
        buf.initialize(1,1,1)
        buf.setCentreFreq(0, 1000)
        #initialise model
        model.initialize(buf)
        #final output should be loudness
        moduleIndex = None
        if moduleIndex is None:
            moduleIndex = model.getNModules()-1
        outputBank = model.getModuleOutput(moduleIndex)
    outputSones = np.zeros(sones.size)

    for i, phon in enumerate(phons):
        if(model.isDynamicModel()):
            tone = Sound.tone(1000, dur=0.6,fs=fs)
            tone.ref = 2e-5
            tone.normalise(phon)
            tone.applyRamp(0.1) 
            extractor.process(tone.data[:,0])
            #For 1kHz tone, global loudness can be estimated as:
            outputSones[i] = extractor.computeGlobalLoudness(0.2,0.4,'MAX')[0]
        else:
            buf.setSample(0,0,10**(phon/10.0))
            model.process(buf)
            outputSones[i] = outputBank.getSample(0,0)
            #reset model state 
            model.reset()
        print "Input level (dB SPL):", phon
        print "Loudness (sones):", outputSones[i]
        
    #Compute relative error as %
    relativeError = 100 * np.abs(sones - outputSones) / sones

    return outputSones, relativeError

if __name__ == '__main__':

    fs = 32000
    model = ln.SteadyLoudnessANSIS3407()
    #model = ln.DynamicLoudnessGM("../../filterCoefs/32000_FIR_4096_freemid.npy")
    #model.loadParameterSet(0)
    #model.setAnsiSpecificLoudness(True)
    loudness1kHz(model, fs)
