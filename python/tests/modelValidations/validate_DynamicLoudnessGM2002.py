import matplotlib.pylab as plt
import numpy as np
import loudness as ln
import sys,os
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)),'../'))
from sound import Sound
from loudnessExtractor import LoudnessExtractor

def pureToneLoudness(levels = 40, freq = 1000):
    '''
    Computes the loudness of a pure tone at a speified frequency and level. 
    If levels is a list, the loudness will be computed for each level.
    '''
    if type(levels) in [int, float]:
        levels = [levels]

    #Model config
    pathToFilterCoefs = '../../../filterCoefs/32000_FIR_4096_freemid.npy'
    model = ln.DynamicLoudnessGM2002(pathToFilterCoefs)
    #model.loadParameterSet("GM2002")
    model.setRate(250)
    fs = 32000
    #Use LoudnessExtractor to compute the global loudness
    extractor = LoudnessExtractor(model, fs, 'LongTermLoudness')
    #output LTL only
    outputLoudness = np.zeros((len(levels), 1))

    for i, level in enumerate(levels):
        tone = Sound.tone(freq, dur = 2.0, fs = fs)
        tone.useDBSPL()
        tone.normalise(level, "RMS")
        tone.applyRamp(0.1)
        extractor.process(tone.data)
        extractor.computeGlobalLoudnessFeatures('LongTermLoudness', 0.7, 2.0)
        outputLoudness[i] = extractor.outputDict['Features']['LongTermLoudness']['Mean']
        print "Tone frequency:", freq
        print "Input level (dB SPL):", level
        print "Loudness:", outputLoudness[i]

    return outputLoudness 

def writeToCSVFile(expected, measured, filename):
    np.savetxt(filename, np.hstack(( 
        expected.reshape((-1, 1)),\
        measured.reshape((-1, 1)))), \
        delimiter = ',', header = 'Expected, Measured', comments = '')
    
if __name__ == '__main__':

    '''
    Pure tone tests
    '''
    #Table 7
    levels1kHz = np.array([0,1,2,3,4,5,7.5])
    levels1kHz = np.append(levels1kHz, np.arange(10, 125, 5))
    expected1kHz = np.array([0.0011, 0.0018, 0.0028, 0.0044, 0.0065, 0.0088, 0.017,
        0.029, 0.070, 0.142, 0.255, 0.422, 0.662, 0.997, 1.461, 2.098, 2.970,
        4.166, 5.813, 8.102, 11.326, 15.980, 22.929, 33.216, 48.242, 70.362,
        103.274, 152.776, 227.855, 341.982])

    #Pure tones: Example 2
    levels3kHz = np.array([20, 40, 60, 80]);
    expected3kHz = np.array([0.05, 0.41, 1.75, 6.67])

    #Pure tones: Example 4 
    levels100Hz = np.array([50])
    expected100Hz = np.array([0.345])

    #Run pure tone tests
    loudness = pureToneLoudness(levels1kHz, 1000)
    writeToCSVFile(expected1kHz, loudness, './data/DynamicLoudnessGM_1kHz.csv')
    loudness = pureToneLoudness(levels3kHz, 3000)
    writeToCSVFile(expected3kHz, loudness, './data/DynamicLoudnessGM_3kHz.csv')
    loudness = pureToneLoudness(levels100Hz, 100)
    writeToCSVFile(expected100Hz, loudness, './data/DynamicLoudnessGM_100Hz.csv')
