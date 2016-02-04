import numpy as np
import matplotlib.pyplot as plt
from sound import Sound
from iterators import StationaryLoudnessIterator, DynamicLoudnessIterator
from scipy.interpolate import interp1d

# ISO 389-7 - free-field values
freqsISO389 = np.array([20.0, 25, 31.5, 40, 50, 63, 80, 100, 125, 160, 200,
                        250, 315, 400, 500, 630, 750, 800, 1000, 1250, 1500,
                        1600, 2000, 2500, 3000, 3150, 4000, 5000, 6000, 6300,
                        8000, 9000, 10000, 11200, 12500, 14000, 16000, 18000])

thresholdsISO389 = np.array([78.5, 68.7, 59.5, 51.1, 44, 37.5, 31.5, 26.5,
                             22.1, 17.9, 14.4, 11.4, 8.6, 6.2, 4.4, 3, 2.4,
                             2.2, 2.4, 3.5, 2.4, 1.7, -1.3, -4.2, -5.8, -6.0,
                             -5.4, -1.5, 4.3, 6, 12.6, 13.9, 13.9, 13, 12.3,
                             18.4, 40.2, 73.2])


class StationaryLoudnessContourPredictor():

    def __init__(self,
                 model,
                 outputName,
                 loudnessLevelFunction=None,
                 loudnessLevel='abs',
                 useISO2261987=False):

        self.iterator = StationaryLoudnessIterator(
            model,
            outputName,
            loudnessLevelFunction
        )

        self.targetLoudnessLevel = loudnessLevel
        self.tol = 0.02
        self.nIters = 20
        self.predictions = None
        self.converged = False
        self.alpha = 0.5

        if ((type(loudnessLevel) is str) or (loudnessLevel == 2.4)):
            self.freqs = freqsISO389
            self.sPLs = thresholdsISO389
            self.targetLoudnessLevel = 2.4
        else:
            if useISO2261987:
                contour = ISO2261987LoudnessContours()
            else:
                contour = ISO2262003LoudnessContours()
            self.freqs = contour.freqs
            self.sPLs = contour.phonToSPL(None, loudnessLevel)

    def process(self):

        self.predictions = np.zeros(self.freqs.size)
        self.converged = np.zeros(self.freqs.size, dtype=bool)
        for i, freq in enumerate(self.freqs):
            print 'Freq: %0.2f, initial guess: %0.2f' % (freq, self.sPLs[i])
            levels = np.zeros(self.freqs.size) - 100
            levels[i] = self.sPLs[i]
            self.predictions[i] = self.sPLs[i]
            self.predictions[i] += self.iterator.process(
                self.freqs,
                levels,
                None,
                self.targetLoudnessLevel,
                self.tol,
                self.nIters,
                self.alpha
            )
            self.converged[i] = self.iterator.converged

    def plotPredictions(self):

        plt.semilogx(self.freqs, self.sPLs, label='ISO target')
        plt.semilogx(
            self.freqs, self.predictions, color='r',
            linestyle='--', label='Predicted')
        plt.legend()
        plt.show()

    def computeErrors(self):
        '''
        Returns a tuple of error vector (target - prediction), RMSE and maximum
        absolute error.
        '''
        error = self.sPLs - self.predictions
        rMSE = np.sqrt(np.mean(error * error))
        maxE = np.max(np.abs(error))
        return (error, rMSE, maxE)

    def getResults(self):
        outputDic = {}
        outputDic['Errors'], outputDic['RMSE'], outputDic['MXAE'] = \
            self.computeErrors()

        outputDic['Frequencies'] = self.freqs
        outputDic['Targets'] = self.sPLs
        outputDic['Predictions'] = self.predictions
        outputDic['Converged'] = self.converged
        return outputDic


class DynamicLoudnessContourPredictor():
    def __init__(self,
                 model,
                 fs,
                 outputName,
                 globalLoudnessFeature=None,
                 loudnessLevelFunction=None,
                 loudnessLevel='abs',
                 useISO2261987=False):

        self.iterator = DynamicLoudnessIterator(
            model,
            fs,
            outputName,
            globalLoudnessFeature,
            loudnessLevelFunction
        )

        self.targetLoudnessLevel = loudnessLevel
        self.tol = 0.02
        self.nIters = 20
        self.predictions = None
        self.converged = False
        self.fs = fs
        self.duration = 1
        self.alpha = 0.5

        if ((type(loudnessLevel) is str) or (loudnessLevel == 2.4)):
            self.freqs = freqsISO389
            self.sPLs = thresholdsISO389
            self.targetLoudnessLevel = 2.4
        else:
            if useISO2261987:
                contour = ISO2261987LoudnessContours()
            else:
                contour = ISO2262003LoudnessContours()
            self.freqs = contour.freqs
            self.sPLs = contour.phonToSPL(None, loudnessLevel)
        self.predictions = np.zeros(self.sPLs.size)

    def process(self):

        self.predictions = np.zeros(self.freqs.size)
        self.converged = np.zeros(self.freqs.size, dtype=bool)
        for i, freq in enumerate(self.freqs):
            print 'Freq: %0.2f, initial guess: %0.2f' % (freq, self.sPLs[i])
            s = Sound.tone([freq], dur=self.duration, fs=self.fs)
            s.applyRamp(0.1)
            s.useDBSPL()
            s.normalise(self.sPLs[i], 'RMS')

            self.predictions[i] = self.sPLs[i]
            self.predictions[i] += self.iterator.process(
                s.data,
                self.targetLoudnessLevel,
                self.tol,
                self.nIters,
                self.alpha
            )
            self.converged[i] = self.iterator.converged

    def plotPredictions(self):

        plt.semilogx(
            self.freqs, self.sPLs, label='ISO'
        )
        plt.semilogx(
            self.freqs, self.predictions, color='r',
            linestyle='--', label='Predicted'
        )
        plt.legend()
        plt.show()

    def computeErrors(self):
        '''
        Returns a tuple of error vector (target - prediction), RMSE and maximum
        absolute error.
        '''
        error = self.sPLs - self.predictions
        rMSE = np.sqrt(np.mean(error * error))
        maxE = np.max(np.abs(error))
        return (error, rMSE, maxE)

    def getResults(self):
        outputDic = {}
        outputDic['Errors'], outputDic['RMSE'], outputDic['MXAE'] = \
            self.computeErrors()

        outputDic['Frequencies'] = self.freqs
        outputDic['Targets'] = self.sPLs
        outputDic['Predictions'] = self.predictions
        outputDic['Converged'] = self.converged
        return outputDic


class ISO2262003LoudnessContours():
    '''
    At present this class generates:
    1. Loudness levels in phons given an array of frequencies
       and the corresponding SPL values.
    2. SPLs given an array of frequencies and the corresponding
       loudness levels in phon.

    For example:

    contour = ISOContours()
    spls = contour.phonToSPL(arange(20,4000), 70)
    phons = contour.sPLToPhon(arange(20,4000), spls)

    To view the standardised a set of contours:
    contour.PlotContours()
    '''
    def __init__(self):

        self.freqs = np.array([
            20, 25, 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400,
            500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000,
            6300, 8000, 10000, 12500
        ])

        self.l_params = np.array([
            -31.6, -27.2, -23, -19.1, -15.9, -13, -10.3, -8.1, -6.2,
            -4.5, -3.1, -2.0, -1.1, 0.4, 0.0, 0.3, 0.5, 0.0, -2.7,
            -4.1, -1.0, 1.7, 2.5, 1.2, -2.1, -7.1, -11.2, -10.7, -3.1
        ])
        self.t_params = np.array([
            78.5, 68.7, 59.5, 51.1, 44.0, 37.5, 31.5, 26.5, 22.1, 17.9,
            14.4, 11.4, 8.6, 6.2, 4.4, 3.0, 2.2, 2.4, 3.5, 1.7, -1.3,
            -4.2, -6.0, -5.4, -1.5, 6.0, 12.6, 13.9, 12.3
        ])
        self.alpha_params = np.array([
            0.532, 0.506, 0.480, 0.455, 0.432, 0.409, 0.387, 0.367,
            0.349, 0.330, 0.315, 0.301, 0.288, 0.276, 0.267, 0.259, 0.253,
            0.250, 0.246, 0.244, 0.243, 0.243, 0.243, 0.242, 0.242, 0.245,
            0.254, 0.271, 0.301
        ])

    def lu_dB(self, f):
        return interp1d(self.freqs, self.l_params, 'cubic')(f)

    def t_dB(self, f):
        return interp1d(self.freqs, self.t_params, 'cubic')(f)

    def alpha(self, f):
        return interp1d(self.freqs, self.alpha_params, 'cubic')(f)

    def bf(self, l, t, a, level):
        return (0.4 * 10 ** (((level + l) / 10.0) - 9)) ** \
            a - (0.4 * 10 ** (((t + l) / 10.0) - 9)) ** a + 0.005135

    def af(self, l, t, a, ln):
        return 4.47e-3 * (10 ** (0.025 * ln) - 1.15) + \
            (0.4 * 10 ** (((t + l) / 10.0) - 9)) ** a

    def phonToSPL(self, f, phon):
        if f is None:
            f = self.freqs.copy()
        l = self.lu_dB(f)
        t = self.t_dB(f)
        a = self.alpha(f)
        spls = (np.log10(self.af(l, t, a, phon)) *
                10.0 / self.alpha(f)) - self.lu_dB(f) + 94.0
        return spls

    def sPLToPhon(self, f, level):
        if f is None:
            f = self.freqs
        l = self.lu_dB(f)
        t = self.t_dB(f)
        a = self.alpha(f)
        phons = 40.0 * np.log10(self.bf(l, t, a, level)) + 94.0
        return phons

    def plotContours(self, phonLevels=None, ax=None):

        if not ax:
            fig, ax = plt.subplots(figsize=(6, 4))
        if phonLevels is None:
            phonLevels = np.array([10, 20, 30, 40, 50, 60, 70, 80, 90, 100])

        for phon in phonLevels:
            ax.semilogx(self.freqs, self.phonToSPL(None, phon))
        plt.xlabel("Frequency, Hz")
        plt.ylabel("Level, dB SPL")
        plt.xlim((16, 16e3))
        plt.ylim((-10, 130))
        plt.show()


class ISO2261987LoudnessContours():

    def __init__(self):

        self.freqs = np.array([20, 25, 31.5, 40, 50, 63, 80, 100, 125, 160,
                               200, 250, 315, 400, 500, 630, 800, 1000, 1250,
                               1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000,
                               10000, 12500])

        self.afParams = np.array([2.347, 2.190, 2.050, 1.879, 1.724, 1.597,
                                  1.512, 1.466, 1.426, 1.394, 1.372, 1.344,
                                  1.304, 1.256, 1.203, 1.135, 1.062, 1.0,
                                  0.967, 0.943, 0.932, 0.933, 0.937, 0.952,
                                  0.974, 1.027, 1.135, 1.266, 1.501])

        self.bfParams = np.array([0.00561, 0.00527, 0.00481, 0.00404, 0.00338,
                                  0.00286, 0.00259, 0.00257, 0.00256, 0.00255,
                                  0.00254, 0.00248, 0.00229, 0.00201, 0.00162,
                                  0.00111, 0.00052, 0, -0.00039, -0.00067,
                                  -0.00092, -0.00105, -0.00104, -0.00088,
                                  -0.00055, 0.00000, 0.00089, 0.00211,
                                  0.00488])
        self.tfParams = np.array([74.3, 65.0, 56.3, 48.4, 41.7, 35.5, 29.8,
                                  25.1, 20.7, 16.8, 13.8, 11.2, 8.9, 7.2, 6.0,
                                  5.0, 4.4, 4.2, 3.7, 2.6, 1.0, -1.2, -3.6,
                                  -3.9, -1.1, 6.6, 15.3, 16.4, 11.6])

    def af(self, f):
        return interp1d(self.freqs, self.afParams, 'cubic')(f)

    def bf(self, f):
        return interp1d(self.freqs, self.bfParams, 'cubic')(f)

    def tf(self, f):
        return interp1d(self.freqs, self.tfParams, 'cubic')(f)

    def phonToSPL(self, f, phon):
        if f is None:
            f = self.freqs.copy()
        af = self.af(f)
        bf = self.bf(f)
        tf = self.tf(f)
        num = 5.0 * tf * (af - bf * phon + bf * 4.2) + 5.0 * phon - 21.0
        den = 5.0 * (af - bf * phon) + 21.0 * bf
        spl = num / den
        return spl

    def sPLToPhon(self, f, spl):
        if f is None:
            f = self.freqs
        af = self.af(f)
        bf = self.bf(f)
        tf = self.tf(f)
        phons = 4.2 + af * (spl - tf) / (1.0 + bf * (spl - tf))
        return phons

    def plotContours(self, phonLevels=None):

        if phonLevels is None:
            phonLevels = np.array([10, 20, 30, 40, 50, 60, 70, 80, 90, 100])

        for phon in phonLevels:
            plt.semilogx(self.freqs, self.phonToSPL(None, phon))
        plt.xlabel("Frequency, Hz")
        plt.ylabel("Level, dB SPL")
        plt.xlim((16, 16e3))
        plt.ylim((-10, 130))
        plt.show()
