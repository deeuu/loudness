import numpy as np

nominalThirdOctBandFC = np.array([50, 62.5, 80, 100, 125, 160, 200, 250, 315,
                                  400, 500, 630, 800, 1000, 1250, 1600, 2000,
                                  2500, 3150, 4000, 5000, 6300, 8000, 10000,
                                  12700, 16000])


def getF1AndF2(fc, bw, fcIsGeometricMean=True):
    if fcIsGeometricMean:
        f2 = 0.5 * (bw + np.sqrt(bw ** 2 + 4 * fc ** 2))
        f1 = f2 - bw
    else:
        f2 = fc + bw * 0.5
        f1 = fc - bw * 0.5
    return (f1, f2)


def generateNToneComplex(fc,
                         bw,
                         nTones,
                         dBSPL,
                         overallLevel=True,
                         fcIsGeometricMean=True):

    f1, f2 = getF1AndF2(fc, bw, fcIsGeometricMean)
    return generateNToneComplexBand(f1,
                                    f2,
                                    nTones,
                                    dBSPL,
                                    overallLevel,
                                    fcIsGeometricMean)


def generateNToneComplexBand(f1,
                             f2,
                             nTones,
                             dBSPL,
                             overallLevel=True,
                             logSpaced=True):
    '''
    Generates an nTone complex including at minimum the f1 frequency.
    If nTones is > 1, both f1 and f2 are included.
    By default, the components are spaced logarithmically.
    '''
    if logSpaced:
        componentFreqs = np.logspace(np.log10(f1), np.log10(f2), nTones)
    else:
        componentFreqs = np.linspace(f1, f2, nTones)

    if overallLevel:
        componentIntensities = np.full(nTones, 10 ** (dBSPL / 10.0) / nTones)
    else:
        componentIntensities = np.full(nTones, 10 ** (dBSPL / 10.0))

    return componentFreqs, componentIntensities


def generateWhiteNoiseBand(f1,
                           f2,
                           dBSPL,
                           overallLevel=True,
                           spacing=None):
    '''
    In the standard the band frequences don't seem to be included i.e. (f1, f2)
    Here we do: [f1 + spacing/2, f2 + spacing/2]
    '''
    if not spacing:
        if (f2 - f1) > 30:
            spacing = 10
            f1 += 5
        else:
            spacing = 1
            f1 += 0.5

    componentFreqs = np.arange(f1, f2, spacing)
    nComponents = componentFreqs.size
    if overallLevel:
        componentIntensities = np.full(nComponents,
                                       10 ** (dBSPL / 10.0) / nComponents)
    else:
        componentIntensities = np.full(nComponents,
                                       10 ** (dBSPL / 10.0) * spacing)

    return componentFreqs, componentIntensities


def generatePinkNoiseBand(f1,
                          f2,
                          level,
                          refFreq=None,
                          spacing=None):
    '''
    if `refFreq' is not None, i.e. a frequency in Hz,
    `level' is taken to be the spectrum level at that frequency.
    Otherwise, `level' is taken to be the overall level of the band.
    '''
    if not spacing:
        if (f2 - f1) > 30:
            spacing = 10
            f1 += 5
        else:
            spacing = 1
            f1 += 0.5

    componentFreqs = np.arange(f1, f2, spacing)
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
        f, ints = generateWhiteNoiseBand(lower[i],
                                         upper[i],
                                         bandLevels[i],
                                         True)
        freqs = np.hstack((freqs, f))
        componentIntensities = np.hstack((componentIntensities, ints))
    return freqs, componentIntensities
