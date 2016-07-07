import numpy as np
import matplotlib.pyplot as plt

class RoexBankANSIS342007():

    def __init__(
            self,
            camLo=1.8, 
            camHi=38.9, 
            camStep=0.1):

        self.camLo = camLo
        self.camHi = camHi
        self.camStep = camStep 

    def camToFreq(self, cam):
        return (10**(cam/21.366)-1)/4368e-6

    def freqToERB(self, freq):
        return 24.673 * (4368e-6 * freq + 1)
    
    def initialize(self, freqs):

        # Main filter centre freqs
        self.fcCams = np.arange(
                    self.camLo, 
                    self.camHi + self.camStep,
                    self.camStep,
                    ).reshape(-1, 1)

        self.fc = self.camToFreq(self.fcCams)
                
        # Store input freqs for plotting
        self.freqs = freqs.astype('float').reshape((-1, 1))
        tiledFreqs = np.tile(self.freqs.T, (self.fc.size, 1))
        tiledCompFreqs = np.tile(self.freqs.T, (freqs.size, 1))

        # Slopes of symmetrical filters
        compErbs = self.freqToERB(self.freqs)
        compP = 4.0 * self.freqs / compErbs
        
        # Compute the fixed roex filters
        compG = np.abs(tiledCompFreqs - self.freqs) / self.freqs
        compPg = compP * compG
        self.wComp = (1 + compPg) * np.exp(-compPg)
        self.wComp[compG > 2] = 0.0

        # Params for level dependent roex filters
        self.g = (tiledFreqs - self.fc) / self.fc
        self.g2GT0 = self.g > 0
        self.g2GT2 = self.g > 2
        self.g = np.abs(self.g)
        self.roexFilters = np.zeros(self.g.shape)
        erbs = self.freqToERB(self.fc)
        p51_1k = 4.0 * 1000.0 / self.freqToERB(1000.0)
        self.pu = np.tile(4.0 * self.fc / erbs, self.freqs.size)
        self.pl = 0.35 * (self.pu / p51_1k);

        self.initialized = True
        
    def process(self,psd):
        
        # Power per ERB
        self.wCompOut = 10.0 * np.log10(np.dot(self.wComp, psd) + 1e-10)

        p = self.pu - (self.pl * (self.wCompOut - 51.0));
        p[self.g2GT0] = self.pu[self.g2GT0]
        p[p < 0.1] = 0.1
        pg = p * self.g
        self.roex = (1.0 + pg) * np.exp(-pg)
        self.roex[self.g2GT2] = 0.0

        self.excitation = np.dot(self.roex, psd)

    def plotFirstStageFilters(self, colour = 'k'):
        plt.semilogx(self.freqs, 10 * np.log10(self.wComp.T + 1e-5), colour)
        plt.xlabel('Frequency, Hz')
        plt.ylabel('Response, dB')
        plt.show()

    def plotSecondStageFilters(self, colour = 'k'):
        plt.semilogx(self.freqs, 10 * np.log10(self.roex.T + 1e-10), colour)
        plt.xlabel('Frequency, Hz')
        plt.ylabel('Response, dB')
        plt.show()

    def plotExcitationPattern(self, colour = 'k'):
        plt.semilogx(self.fc, 10*np.log10(self.excitation + 1e-10), colour)
        plt.xlabel('Centre frequency, Hz')
        plt.ylabel('Excitation level, dB')
        plt.xlim(100,20000)
        plt.ylim(0,100)
        plt.show()

if __name__ == '__main__':
    freq = 1000.0
    f = np.arange(500, 2000.0, 10)
    psd = np.zeros(f.size)
    psd[np.abs(freq-f).argmin()] = 10 ** (90 / 10.0)
    roex = RoexBankANSIS342007(2.5, 40, 1.0)
    roex.initialize(f)
    roex.process(psd)
    roex.plotFirstStageFilters()
    roex.plotSecondStageFilters()
    roex.plotExcitationPattern()
