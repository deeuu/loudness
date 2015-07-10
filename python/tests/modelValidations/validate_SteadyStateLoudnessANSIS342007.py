import numpy as np
import loudness as ln
import sys,os
sys.path.append('../../tools/')
from usefulFunctions import *
from extractors import StationaryLoudnessExtractor
import matplotlib.pyplot as plt

if __name__ == '__main__':

    model = ln.SteadyStateLoudnessANSIS342007()
    feature = 'InstantaneousLoudness'
    extractor = StationaryLoudnessExtractor(model, feature)

    '''
    Pure tone tests
    '''
    #Table 7
    levels = np.array([0, 1, 2, 3, 4, 5, 7.5])
    levels = np.append(levels, np.arange(10, 125, 5))
    expected = np.array([0.0011, 0.0018, 0.0028, 0.0044, 0.0065, 0.0088, 0.017,
        0.029, 0.070, 0.142, 0.255, 0.422, 0.662, 0.997, 1.461, 2.098, 2.970,
        4.166, 5.813, 8.102, 11.326, 15.980, 22.929, 33.216, 48.242, 70.362,
        103.274, 152.776, 227.855, 341.982])

    measured = np.zeros(expected.size)
    for i, level in enumerate(levels):
        extractor.process(np.array([1000.0]), np.array([level]))
        measured[i] = extractor.outputDict[feature]
    writeToCSVFile(levels, expected, measured, './data/SteadyStateLoudnessANSIS342007_1kHz.csv')

    #Pure tones: Example 2
    levels = np.array([20, 40, 60, 80]);
    expected = np.array([0.35, 1.8, 7.1, 27.5])
    measured = np.zeros(expected.size)
    for i, level in enumerate(levels):
        extractor.process(np.array([3000.0]), np.array([level]))
        measured[i] = extractor.outputDict[feature]
    writeToCSVFile(levels, expected, measured,
            './data/SteadyStateLoudnessANSIS342007_3kHz.csv')

    #Pure tones: Example 4 
    levels = np.array([50])
    expected = np.array([0.345])
    measured = np.zeros(expected.size)
    for i, level in enumerate(levels):
        extractor.process(np.array([100.0]), np.array([level]))
        measured[i] = extractor.outputDict[feature]
    writeToCSVFile(levels, expected, measured, './data/SteadyStateLoudnessANSIS342007_100Hz.csv')

    '''
    Filtered noise tests
    '''
    #Example1:
    expected = np.array([4.25, 14.29])
    measured = np.zeros(2)

    freqs, spectrum = generateWhitenoiseBandwidth(1000, 100, 40, False)
    extractor.process(freqs, 10 * np.log10(spectrum))
    measured[0] = extractor.outputDict[feature]
    freqs, spectrum = generateWhitenoiseBandwidth(1000, 1000, 40, False)
    extractor.process(freqs, 10 * np.log10(spectrum))
    measured[1] = extractor.outputDict[feature]
    writeToCSVFile(np.array([100, 1000]), expected, measured,
        './data/SteadyStateLoudnessANSIS342007_whitenoiseEX1.csv')

    #Example2:
    expected = np.array([4.25, 8.02])
    measured = np.zeros(2)

    freqs, spectrum = generateWhitenoiseBandwidth(1000, 100, 40, True)
    extractor.process(freqs, 10 * np.log10(spectrum))
    measured[0] = extractor.outputDict[feature]
    freqs, spectrum = generateWhitenoiseBandwidth(1000, 1000, 40, True)
    extractor.process(freqs, 10 * np.log10(spectrum))
    measured[1] = extractor.outputDict[feature]
    writeToCSVFile(np.array([100, 1000]), expected, measured,
            './data/SteadyStateLoudnessANSIS342007_whitenoiseEX2.csv')

    #Example3:
    levels = np.array([0, 20, 40])
    expected = np.array([3.62, 16.00, 49.28])
    measured = np.zeros(3)

    for i, level in enumerate(levels):
        freqs, spectrum = generatePinknoise(50, 15000, level, 1000)
        extractor.process(freqs, 10 * np.log10(spectrum))
        measured[i] = extractor.outputDict[feature]

    writeToCSVFile(levels, expected, measured,
            './data/SteadyStateLoudnessANSIS342007_pinknoiseEX3.csv')

    #Example4:
    levels = np.arange(0, 60, 10)
    expected = np.array([0.071, 0.67, 2.51, 6.26, 12.7, 23.3])
    measured = np.zeros(levels.size)

    for i, level in enumerate(levels):
        bandLevels = np.ones(26) * level
        freqs, spectrum = generateSpectrumFromThirdOctaveBandLevels(bandLevels) 
        extractor.process(freqs, 10 * np.log10(spectrum))
        measured[i] = extractor.outputDict[feature]

    writeToCSVFile(levels, expected, measured,
            './data/SteadyStateLoudnessANSIS342007_thirdOctaveEX4.csv')
    '''
    Multiple tones
    '''
    level = 60
    expected = np.array([6.35])
    measured = np.zeros(expected.size)
    extractor.process(np.array([1500, 1600, 1700]), np.array([level, level, level]))
    measured[0] = extractor.outputDict[feature]
    writeToCSVFile(np.array([level]), expected, measured, './data/SteadyStateLoudnessANSIS342007_multiTones1.csv')

    level = 60
    expected = np.array([12.62])
    measured = np.zeros(expected.size)
    extractor.process(np.array([1500, 1600, 2400]), np.array([level, level, level]))
    measured[0] = extractor.outputDict[feature]
    writeToCSVFile(np.array([level]), expected, measured, './data/SteadyStateLoudnessANSIS342007_multiTones2.csv')

    level = 30
    expected = np.array([1.99])
    measured = np.zeros(expected.size)
    extractor.process(np.arange(100, 1100, 100), np.ones(10) * level)
    measured[0] = extractor.outputDict[feature]
    writeToCSVFile(np.array([level]), expected, measured, './data/SteadyStateLoudnessANSIS342007_multiTones3.csv')

    '''
    Tones plus noise
    '''
    expected = np.array([5.14])
    measured = np.zeros(expected.size)
    
    freqs, spectrum = generateWhitenoiseBandwidth(1000, 100, 40, False)
    idx = np.where(freqs == 1000)[0]
    spectrum[idx] += 10 ** (60 / 10.0)
    extractor.process(freqs, 10 * np.log10(spectrum))
    measured[0] = extractor.outputDict[feature]
    writeToCSVFile(np.array([40]), expected, measured,
            './data/SteadyStateLoudnessANSIS342007_tonePlusNoise.csv')
