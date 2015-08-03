import numpy as np

nominalThirdOctBandFC = np.array([50, 62.5, 80, 100, 125, 160, 200, 250, 315,
    400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300,
    8000, 10000, 12700, 16000])

def generateWhiteNoiseBandFromFc(centreFreq, bandwidth, dBSPL, overallLevel=True):
    '''See 3.2 b) of ANSI S3.4 2007.
    '''
    bandLo = centreFreq - (bandwidth / 2.0)
    bandHi = centreFreq + (bandwidth / 2.0)

    return generateWhiteNoiseBand(bandLo, bandHi, dBSPL, overallLevel)

def generateWhiteNoiseBand(bandLo, bandHi, dBSPL, overallLevel = True, spacing = None):
    '''
    In the standard the band frequences don't seem to be included i.e. (bandLo, bandHi)
    Here we do: [bandLo + spacing/2, bandHi + spacing/2]
    '''

    if not spacing:
        if (bandHi - bandLo) > 30:
            spacing = 10
            bandLo += 5
        else:
            spacing = 1
            bandLo += 0.5

    componentFreqs = np.arange(bandLo, bandHi, spacing)
    nComponents = componentFreqs.size
    if overallLevel:
        componentIntensities = np.full(nComponents, 10**(dBSPL / 10.0) / nComponents)
    else:
        componentIntensities = np.full(nComponents, 10**(dBSPL / 10.0) * spacing)

    return componentFreqs, componentIntensities

def generatePinkNoise(bandLo, bandHi, spectrumLevel, refFreq):

    if (bandHi - bandLo) > 30:
        spacing = 10
        bandLo += 5
    else:
        spacing = 1
        bandLo += 0.5

    componentFreqs = np.arange(bandLo, bandHi, spacing)
    gain = spacing * 10**(spectrumLevel / 10.0) * refFreq
    componentIntensities = gain / componentFreqs
    return componentFreqs, componentIntensities

def generateSpectrumFromThirdOctaveBandLevels(bandLevels):
    '''
    Components are uniformly spaced over each third octave band.
    '''
    if bandLevels.size != nominalThirdOctBandFC.size:
        raise ValueError("Must provide 26 levels")
    lower = nominalThirdOctBandFC / 2 ** (1 / 6.0)
    upper = nominalThirdOctBandFC * 2 ** (1 / 6.0)
    freqs = np.array([])
    componentIntensities = np.array([])
    for i, level in enumerate(bandLevels):
        f, ints = generateWhiteNoiseBand(lower[i], upper[i], 
                bandLevels[i], True)
        freqs = np.hstack((freqs, f))
        componentIntensities = np.hstack((componentIntensities, ints))
    return freqs, componentIntensities
