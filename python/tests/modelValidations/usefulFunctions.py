import numpy as np

thirdOctBands = np.array([49.6, 62.5, 78.7, 99.2, 125, 157.5, 198.4, 250, 315, 396.9, 500,
630, 793.7, 1000, 1259.9, 1587.4, 2000, 2519.8, 3174.8, 4000, 5039.7, 6349.6,
8000, 10079.4, 12699.2, 16000])

def generateWhitenoiseBandwidth(centreFreq, bandwidth, dBSPL, overallLevel=True):
    '''See 3.2 b) of ANSI S3.4 2007.
    '''
    bandLo = centreFreq - (bandwidth / 2.0)
    bandHi = centreFreq + (bandwidth / 2.0)

    return generateWhitenoiseBands(bandLo, bandHi, dBSPL, overallLevel)

def generateWhitenoiseBands(bandLo, bandHi, dBSPL, overallLevel = True, spacing = None):
    '''
    In the standard the band frequences don't seem to be included i.e. (bandLo, bandHi)
    Here we do: [bandLo, bandHi)
    Thus this could give subtle differences for examples in which the spectrum
    level is used (rather than the overall level).
    '''

    if not spacing:
        if (bandHi - bandLo) > 30:
            spacing = 10
        else:
            spacing = 1

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

def generateSpectrumFromThirdOctaveBandLevels(bandLevels):
    '''
    Components are uniformly spaced over each third octave band.
    '''
    if bandLevels.size != thirdOctBands.size:
        raise ValueError("Need to provide 26 band levels")
    lower = thirdOctBands / 2 ** (1 / 6.0)
    upper = thirdOctBands * 2 ** (1 / 6.0)
    freqs = np.array([])
    componentIntensities = np.array([])
    for i, level in enumerate(bandLevels):
        f, ints = generateWhitenoiseBands(lower[i], upper[i], 
                bandLevels[i], True)
        freqs = np.hstack((freqs, f))
        componentIntensities = np.hstack((componentIntensities, ints))
    return freqs, componentIntensities

def writeToCSVFile(param, expected, measured, filename):
    np.savetxt(filename, np.hstack(( 
        param.reshape((-1, 1)),\
        expected.reshape((-1, 1)),\
        measured.reshape((-1, 1)))), \
        delimiter = ',', header = 'Parameter, Expected, Measured', comments = '')
