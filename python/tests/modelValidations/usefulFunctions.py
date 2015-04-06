import numpy as np

def generateWhitenoiseBandwidth(centreFreq, bandwidth, dBSPL, overallLevel=True):
    '''See 3.2 b) of ANSI S3.4 2007.
    '''
    spacing = 1
    if bandwidth > 30:
        spacing = 10
    
    bandLo = centreFreq - (bandwidth / 2.0)
    bandHi = centreFreq + (bandwidth / 2.0)

    return generateWhitenoiseBands(bandLo, bandHi, spacing, dBSPL, overallLevel)

def generateWhitenoiseBands(bandLo, bandHi, spacing, dBSPL, overallLevel = True):
    '''
    In the standard the band frequences don't seem to be included i.e. (bandLo, bandHi)
    Here we do: [bandLo, bandHi)
    Thus this could give subtle differences for examples in which the spectrum
    level is used (rather than the overall level).
    '''
    componentFreqs = np.arange(bandLo, bandHi, spacing)
    nComponents = componentFreqs.size
    if overallLevel:
        componentIntensities = np.full(nComponents, 10**(dBSPL / 10.0) / nComponents)
    else:
        componentIntensities = np.full(nComponents, 10**(dBSPL / 10.0) * spacing)

    return componentFreqs, componentIntensities

def generatePinknoise(bandLo, bandHi, spectrumLevel, refFreq):

    spacing = 1
    if (bandHi - bandLo) > 30:
        spacing = 10

    componentFreqs = np.arange(bandLo, bandHi, spacing)
    gain = spacing * 10**(spectrumLevel / 10.0) * refFreq
    componentIntensities = gain / componentFreqs
    return componentFreqs, componentIntensities

def writeToCSVFile(expected, measured, filename):
    np.savetxt(filename, np.hstack(( 
        expected.reshape((-1, 1)),\
        measured.reshape((-1, 1)))), \
        delimiter = ',', header = 'Expected, Measured', comments = '')
