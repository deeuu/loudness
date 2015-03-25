#!/usr/bin/env

import sys
sys.path.append('../python/tools/')
from sound import Sound

def generatePureTones():
    samplingFreqs = [32000, 44100, 48000]
    freqs = [50, 1000, 3000]
    levels = [40]
    durations = [1.0]

    for fs in samplingFreqs:
        for freq in freqs:
            for dur in durations:
                x = Sound.tone(freq, dur = dur, fs = fs)
                x.ref = 2e-5
                for level in levels:
                    x.normalise(level, "RMS")
                    x.writeToAudioFile('./pureTone_' + str(freq) + 'Hz_' + str(level) +\
                            'dBSPL_' + str(fs) + 'Hz.wav', 'float32')

if __name__ == "__main__":
    generatePureTones()
