import numpy as np

nominalThirdOctBandFC = np.array([50, 62.5, 80, 100, 125, 160, 200, 250, 315,
    400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300,
    8000, 10000, 12700, 16000])

def generateSymmetricNToneComplex(fc, 
                                ratio,
                                nTones, 
                                dBSPL,
                                overallLevel = True,
                                logSpaced = True):
    '''
    Generates an `nTone' complex centred at (and including) fc.
    `nTone' must be odd.
    if `logSpaced' is True (default), components are spaced logarithmically
    according to the frequency `ratio'. If `logSpaced' is False, `ratio' is not
    a ratio, but instead defines the linear spacing between adjacent components in Hertz.
    '''
    if (nTones % 2):
        nTonesBothSides = int(nTones)/2
        ratio = float(ratio)
        if logSpaced:
            fLo = fc / (ratio ** nTonesBothSides)
            fHi = fc * (ratio ** nTonesBothSides)
        else:
            fLo = fc - ratio * nTonesBothSides
            fHi = fc + ratio * nTonesBothSides
        return generateNToneComplex(fLo, fHi, nTones, dBSPL, overallLevel, logSpaced)
    else:
        raise ValueError("nTones must be odd")

def generateNToneComplex(fLo, fHi, nTones, dBSPL, overallLevel = True, logSpaced = True):
    '''
    Generates an nTone complex including at minimum the fLo frequency. 
    If nTones is > 1, both fLo and fHi are included.
    By default, the components are spaced logarithmically.
    '''
    if logSpaced:
        componentFreqs = np.logspace(np.log10(fLo), np.log10(fHi), nTones)
    else:
        componentFreqs = np.linspace(fLo, fHi, nTones)

    if overallLevel:
        componentIntensities = np.full(nTones, 10 ** (dBSPL / 10.0) / nTones)
    else:
        componentIntensities = np.full(nTones, 10 ** (dBSPL / 10.0))

    return componentFreqs, componentIntensities
    
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

def generatePinkNoiseBand(bandLo, bandHi, level, refFreq = None, spacing = None):
    '''
    if `refFreq' is not None, i.e. a frequency in Hz, `level' is taken to be the
    spectrum level at that frequency. Otherwise, `level' is taken to be the
    overall level of the band.  
    '''
    if not spacing:
        if (bandHi - bandLo) > 30:
            spacing = 10
            bandLo += 5
        else:
            spacing = 1
            bandLo += 0.5

    componentFreqs = np.arange(bandLo, bandHi, spacing)
    lin = 10 ** (level / 10.0)
    if refFreq:
        gain = spacing * lin * refFreq
    else:
        gain = lin / np.sum(1.0 / componentFreqs)
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
