import loudness as ln
import numpy as np
import sys
sys.path.append('../tools/')
from predictors import StationaryLoudnessContourPredictor

def func(x):
    return ln.soneToPhonMGB1997(float(x), True)

model = ln.StationaryLoudnessANSIS342007()
predictor = StationaryLoudnessContourPredictor(model, 'InstantaneousLoudness', func, 'abs')
predictor.targetLoudnessLevel = 2.2
predictor.process()
print predictor.computeErrors()
predictor.plotPredictions()
