import loudness as ln
import numpy as np

def ZwickerDIN45631(VectNiv3Oct = None, FieldType = 0):

    '''
    This is a quick and dirty Python port of the Matlab code private by GENESIS ACOUSTICS.

    Input is a numpy array of intensity levels derived from third octave band
    filters with the following centre frequencies:
        [25 31.5 40 50 63 80 100 125 160 200 250 315 400 500 630 800 1000 1250 1600 2000 2500
         3150 4000 5000 6300 8000 10000 12500]

    Output is main loudness, specific loudness, critical band frequencies in
    Bark and total loudness in sones.
    '''

    if FieldType != 1 and FieldType != 0:
        raise ValueError('FieldType value must be 0 or 1. (0=free field, 1=diffuse field)')

    Nbands3Oct=28
    VectNiv3Oct=VectNiv3Oct[:]

    if len(VectNiv3Oct) != Nbands3Oct:
        raise ValueError('Input vector must have ', Nbands3Oct, 'third octave bands values')
    if (np.max(VectNiv3Oct)) > 120 or np.min(VectNiv3Oct) < - 60:
        raise ValueError('Third octave levels must be within interval [-60, 120] dBSPL (for model validity)')

    '''Ranges of 1/3 octave band levels for correction at low frequencies
    according to equal loudness contours'''
    RAP= np.array([45,55,65,71,80,90,100,120])

    '''Reduction of 1/3 octave band levels at low frequencies according to 
    equal loudness contours within the eight ranges defined by RAP'''
    DLL= np.array([[- 32,- 24,- 16,- 10,- 5,0,- 7,- 3,0,- 2,0],
        [- 29,- 22,- 15,-10,- 4,0,- 7,- 2,0,- 2,0],
        [- 27,- 19,- 14,- 9,- 4,0,- 6,- 2,0,- 2,0],
        [-25,- 17,- 12,- 9,- 3,0,- 5,- 2,0,- 2,0],
        [- 23,- 16,- 11,- 7,- 3,0,-4,- 1,0,- 1,0],
        [- 20,- 14,- 10,- 6,- 3,0,- 4,- 1,0,- 1,0],
        [-18,- 12,- 9,- 6,- 2,0,- 3,- 1,0,- 1,0],
        [- 15,- 10,- 8,- 4,-2,0,- 3,- 1,0,- 1,0]])

    '''Critical band rate level at absolute threshold without taking into
    account the transmission characteristics of the ear'''
    LTQ=np.array([30,18,12,8,7,6,5,4,3,3,3,3,3,3,3,3,3,3,3,3])

    '''Correction of levels according to the transmission characteristics of the ear'''
    A0=np.array([0,0,0,0,0,0,0,0,0,0,- 0.5,- 1.6,- 3.2,- 5.4,- 5.6,- 4,-1.5,2,5,12])
    '''Level differences between free and diffuse sound fields'''
    DDF=np.array([0,0,0.5,0.9,1.2,1.6,2.3,2.8,3,2,0,- 1.4,- 2,- 1.9,- 1,0.5,3,4,4.3,4])
    '''Adaptation of 1/3 octave band levels to the corresponding critical band levels'''
    DCB=np.array([- 0.25,- 0.6,- 0.8,- 0.8,-0.5,0,0.5,1.1,1.5,1.7,1.8,1.8,1.7,1.6,1.4,1.2,0.8,0.5,0,- 0.5])
    '''Upper limits of approximated critical bands in terms of critical band rate'''
    ZUP=np.array([0.9,1.8,2.8,3.5,4.4,5.4,6.6,7.9,9.2,10.6,12.3,13.8,15.2,16.7,18.1,19.3,20.6,21.8,22.7,23.6,24])

    '''Range of specific loudness for the determination of the steepness of the
    upper slopes in the specific loudness - critical band rate pattern'''
    RNS=np.array([21.5,18,15.1,11.5,9,6.1,4.4,3.1,2.13,1.36,0.82,0.42,0.3,0.22,0.15,0.1,0.035,0])

    '''Steepness of the upper slopes in the specific loudness - critical band
    rate pattern for the ranges RNS as a function of the number of the
    critical band'''
    USL=np.array([[13,8.2,6.3,5.5,5.5,5.5,5.5,5.5],
        [9,7.5,6,5.1,4.5,4.5,4.5,4.5],
        [7.8,6.7,5.6,4.9,4.4,3.9,3.9,3.9],
        [6.2,5.4,4.6,4.0,3.5,3.2,3.2,3.2],
        [4.5,3.8,3.6,3.2,2.9,2.7,2.7,2.7],
        [3.7,3.0,2.8,2.35,2.2,2.2,2.2,2.2],
        [2.9,2.3,2.1,1.9,1.8,1.7,1.7,1.7],
        [2.4,1.7,1.5,1.35,1.3,1.3,1.3,1.3],
        [1.95,1.45,1.3,1.15,1.1,1.1,1.1,1.1],
        [1.5,1.2,0.94,0.86,0.82,0.82,0.82,0.82],
        [0.72,0.67,0.64,0.63,0.62,0.62,0.62,0.62],
        [0.59,0.53,0.51,0.5,0.42,0.42,0.42,0.42],
        [0.4,0.33,0.26,0.24,0.24,0.22,0.22,0.22],
        [0.27,0.21,0.2,0.18,0.17,0.17,0.17,0.17],
        [0.16,0.15,0.14,0.12,0.11,0.11,0.11,0.11],
        [0.12,0.11,0.1,0.08,0.08,0.08,0.08,0.08],
        [0.09,0.08,0.07,0.06,0.06,0.06,0.06,0.05],
        [0.06,0.05,0.03,0.02,0.02,0.02,0.02,0.02]])

    '''Correction of 1/3 octave band levels according to equal loudness
    contours (XP) and calculation of the intensities for 1/3 octave bands up
    to 315Hz'''
    TI = np.zeros(np.shape(DLL)[1])
    for i in range(np.shape(DLL)[1]):
        j = 0
        while ((VectNiv3Oct[i] > (RAP[j] - DLL[j,i])) and j < 7):
            j = j + 1

        XP = VectNiv3Oct[i] + DLL[j,i]
        TI[i] = 10**(XP / 10.0)

    ''' Determination of levels (LCB) within the first three critical bands'''
    GI = np.zeros(3)
    '''sum of 6 third octave bands from 25 Hz to 80 Hz'''
    GI[0]= np.sum(TI[:6]) 
    '''sum of 3 third octave bands from 100 Hz to 160 Hz'''
    GI[1]= np.sum(TI[6:9])
    '''sum of 2 third octave bands from 200 Hz to 250 Hz'''
    GI[2]= np.sum(TI[9:])

    FNGI = 10 * np.log10(GI)
    LCB = np.zeros((len(GI),1))

    for i in range(len(GI)):
        if GI[i] > 0:
            LCB[i] = FNGI[i]

    '''Calculation of main loudness'''

    Ncriticalbands = 20
    S=0.25
    Ntotalbarkbands=24
    BarkStep=0.1
    LE = np.zeros(Ncriticalbands + 1)
    NM = np.zeros(Ncriticalbands + 1)

    for i in range(Ncriticalbands):

        LE[i]=VectNiv3Oct[i + 8]

        if i <= 2:
            LE[i]=LCB[i]

        LE[i]=LE[i] - A0[i]
        NM[i]=0

        if FieldType == 1:
            LE[i] = LE[i] + DDF[i]

        if LE[i] > LTQ[i]:
            LE[i]=LE[i] - DCB[i]
            MP1 = 0.0635 * 10**(0.025 * LTQ[i])
            MP2 = (1 - S + S * 10**((LE[i] - LTQ[i]) / 10))**0.25 - 1
            NM[i] = MP1 * MP2
            if NM[i] <= 0:
                NM[i]=0

    NM[Ncriticalbands] = 0

    '''correction of specific loudness within the first critical band taking
    into account the dependence of absolute threshold within the band'''
    KORRY = 0.4 + 0.32 * NM[0] ** 0.2

    if KORRY > 1:
        KORRY=1

    '''Initial values settings'''
    NM[0] = NM[0] * KORRY
    N=0.0
    Z1=0.0
    N1=0.0
    IZ=0
    Z=0.1
    NS = np.zeros(np.round(Ntotalbarkbands / BarkStep))

    '''loop over critical bands'''
    for i in range(Ncriticalbands + 1):
        
        ZUP[i] = ZUP[i] + 0.0001

        IG = i - 1
        '''steepness of upper slope (USL) for bands above 8th one are identical'''
        if IG > 7: 
            IG = 7

        while Z1 < ZUP[i]:

            if N1 <= NM[i]:
                '''contribution of unmasked main loudness to total loudness and calculation of values '''
                if N1 < NM[i]:
                    j = 0
                    '''determination of the number j corr to the range of specific loudness'''
                    while (RNS[j] > NM[i]) and (j < 17):
                        j = j + 1

                Z2 = ZUP[i]
                N2 = NM[i]
                N = N + N2 * (Z2 - Z1)
                k = Z

                while (k <= Z2):
                    NS[IZ] = N2
                    IZ = IZ + 1
                    k = k + BarkStep
                Z = k
            else: 
                '''if N1 > NM(i) decision wether the critical band in question
                is completely or partly masked by accessory loudness'''

                N2 = RNS[j]
                if N2 < NM[i]:
                    N2 = NM[i]
                DZ = (N1 - N2) / USL[j,IG]
                Z2 = Z1 + DZ

                if Z2 > ZUP[i]:
                    Z2=ZUP[i]
                    DZ=Z2 - Z1
                    N2=N1 - DZ * USL[j,IG]

                N = N + DZ * (N1 + N2) / 2
                k = Z
                while (k <= Z2):

                    NS[IZ] = N1 - (k - Z1) * USL[j,IG]
                    IZ = IZ + 1
                    k = k + BarkStep

                Z = k

            while (N2 <= RNS[j]) and (j < 17):
                j=j + 1

            if (N2 <= RNS[j]) and (j >= 17):
                j = 17

            Z1 = Z2
            N1 = N2

    '''post-processing'''
    if N < 0:
        N=0
    if N <= 16:
        '''total loudness is rounded to 3 decimals'''
        N = np.floor(N * 1000 + 0.5) / 1000.0 
    else:
        '''total loudness is rounded to 2 decimals'''
        N = np.floor(N * 100 + 0.5) / 100.0 
    N_specif = NS
    N_tot = N
    BarkAxis = np.arange(0.1, Ntotalbarkbands + BarkStep, BarkStep)
    '''LN = gene_sone2phon_ISO532B(N_tot)'''
    return NM, N_specif, BarkAxis, N_tot

# Input signal
minLevel = 40
maxLevel = 80
numPoints = 100
bank = ln.SignalBank()
bank.initialize(1, numPoints, 1, 1,)
bank.setCentreFreqs(np.logspace(1, 4, numPoints))
levels = np.random.rand(1, numPoints, 1) * (maxLevel - minLevel) + minLevel
bank.setSignals(10.0**(levels / 10.0))

# Loudness setup
octBank = ln.OctaveBank(3, 2, True, True)
mainL = ln.MainLoudnessDIN456311991(False)
il = ln.InstantaneousLoudnessDIN456311991()
octBank.addTargetModule(mainL)
mainL.addTargetModule(il)
octBank.initialize(bank)

model = ln.StationaryLoudnessDIN456311991()
model.initialize(bank)

octBankOut = octBank.getOutput()
mainLOut = mainL.getOutput()
ilOut = il.getOutput()
modelOut = model.getOutput('InstantaneousLoudness')

octBank.process(bank)
model.process(bank)

# Python solution
thirdOctLevels = np.squeeze(octBankOut.getSignals())
NM, N_specif, BarkAxis, N_tot = ZwickerDIN45631(thirdOctLevels, 0)

# Compare outputs
print 'Main loudness test:', np.allclose(NM, np.squeeze(mainLOut.getSignals()))
print 'Python solution (loudness in sones): ', N_tot
print 'Loudness module solution (loudness in sones): ', ilOut.getSample(0, 0, 0)
print 'Loudness complete model solution (loudness in sones): ', modelOut.getSample(0, 0, 0)
