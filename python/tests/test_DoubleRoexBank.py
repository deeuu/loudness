import numpy as np
import matplotlib.pyplot as plt
import loudness as ln

class DoubleRoexBank():

    def __init__(self,camLo=1.5, camHi=40.2, camStep=0.1):
        self.camLo = camLo
        self.camHi = camHi
        self.camStep = camStep 
        self.binauralLoudnessFactor = 1.53e-8 * 2 * camStep
        self.initialized = False
        self.processed = True

    def camToFreq(self, cam):
        return (10**(cam/21.366)-1)/4368e-6
    
    def initialize(self, freqs):

        #filter centre freqs
        self.fc = self.camToFreq(np.arange(self.camLo, self.camHi+self.camStep,
            self.camStep))

        #store input freqs for plotting
        self.freqs = freqs.copy()
        tiledFreqs = np.tile(freqs, (self.fc.size, 1)).T

        #slopes
        tl = self.fc/(0.108*self.fc+2.33)
        tu = 15.6
        pl = self.fc/(0.027*self.fc+5.44)
        pu = 27.9
        
        #precalculate some gain terms
        self.maxGdB =  (self.fc/(0.0191*self.fc+1.1)).reshape((self.fc.size, 1))
        self.thirdGainTerm = self.maxGdB/(1+np.exp(0.05*(100-self.maxGdB)))
        
        #compute the fixed filters
        g = (tiledFreqs-self.fc)/self.fc
        pgPassive = tu*g
        pgActive = pu*g

        idx = g<0
        pgPassive[idx] = (-tl*g)[idx]
        pgActive[idx] = (-pl*g)[idx]

        self.wPassive = (1+pgPassive)*np.exp(-pgPassive)
        self.wActive = (1+pgActive)*np.exp(-pgActive)
        self.wPassive[g>2] = 0.0
        self.wActive[g>2] = 0.0

        self.initialized = True
        
    def process(self,psd):
        
        self.pOut = np.dot(self.wPassive.T, psd)

        pOutDB = 10*np.log10(self.pOut + 1e-10)

        self.gain = self.maxGdB - (self.maxGdB /\
                (1 + np.exp(-0.05*(pOutDB-(100-self.maxGdB))))) + self.thirdGainTerm

        idx = pOutDB>30
        self.gain[idx] -= 0.003*(pOutDB[idx]-30)**2
        self.gain = 10**(self.gain/10.0)
    
        self.aOut = np.dot((self.wActive*self.gain.T).T, psd)
        self.excitation = self.pOut + self.aOut

        self.loudness = self.binauralLoudnessFactor*np.sum(self.excitation)

        self.processed = True

    def plotFig3(self):

        maxGainDB = np.arange(20, 70, 10)
        levels = np.tile(np.arange(0,121), (maxGainDB.size,1)).T
        thirdGainTerm = maxGainDB/(1+np.exp(0.05*(100-maxGainDB)))
        gain = maxGainDB - (maxGainDB /\
                (1 + np.exp(-0.05*(levels-(100-maxGainDB))))) + thirdGainTerm

        idx = levels>30
        gain[idx] -= 0.003*(levels[idx]-30)**2

        plt.plot(levels, gain)
        plt.xlabel('Passive output level, dB')
        plt.ylabel('Active gain, dB')

    def plotPassiveFilters(self, colour = 'k'):
        if self.initialized:
            plt.semilogx(self.freqs, 10*np.log10(self.wPassive + 1e-10), colour)
            plt.xlabel('Frequency, Hz')
            plt.ylabel('Response, dB')
            plt.show()

    def plotActiveFilters(self, colour = 'r'):
        if self.initialized:
            plt.semilogx(self.freqs, 10*np.log10(self.wActive+ 1e-10), colour)
            plt.xlabel('Frequency, Hz')
            plt.ylabel('Response, dB')
            plt.show()

    def plotEP(self, colour = 'k'):
        if self.processed:
            plt.semilogx(self.fc, 10*np.log10(self.excitation + 1e-10), colour)
            plt.xlabel('Centre frequency, Hz')
            plt.ylabel('Excitation level, dB')
            plt.xlim(100,20000)
            plt.ylim(0,100)
            plt.show()

if __name__ == '__main__':

    #python side
    fs = 32e3
    N = 2048
    halfPoints = N/2 + 1
    inputFreqs = np.arange(halfPoints)*fs/float(N)

    psd = 10**((20*np.random.randn(halfPoints,1)+70) / 10.0)
    psd /= halfPoints
    '''
    #use for pure tone
    psd = np.zeros((inputFreqs.size, 1))
    k1000 = int(np.round(1000 * (2048 / 32e3)))
    psd[k1000] = 10**(40/10.0)
    '''

    roexbankPy = DoubleRoexBank()
    roexbankPy.initialize(inputFreqs)
    roexbankPy.process(psd)
    excitationPy = roexbankPy.excitation

    #loudness side
    psdLN = ln.SignalBank()
    psdLN.initialize(1, halfPoints, 1, 32000)
    psdLN.setCentreFreqs(inputFreqs)
    psdLN.setSignals(psd.reshape((1, psd.size, 1)))
    bankLN = ln.DoubleRoexBank(1.5, 40.2, 0.1)
    bankLN.initialize(psdLN)
    bankLN.process(psdLN)
    bankLNout = bankLN.getOutput()
    excitationLN = bankLNout.getSignals().flatten()

    print bankLNout.getCentreFreqs()
    plt.semilogx(roexbankPy.fc, 10*np.log10(excitationPy + 1e-10), 'k')
    plt.semilogx(bankLNout.getCentreFreqs(), 10*np.log10(excitationLN + 1e-10), 'r--', marker = 'o')
    plt.show()
    print "Equality test: ", np.allclose(excitationLN,excitationPy[:,0]) 
