import loudness as ln
import numpy as np


def func(x):
    return ln.soneToPhonMGB1997(float(x), True)

model = ln.StationaryLoudnessANSIS342007()
predictor = ln.tools.predictors.StationaryLoudnessContourPredictor(
    model, 'InstantaneousLoudness', func, 'abs')
predictor.targetLoudnessLevel = 2.2
predictor.process()
print predictor.computeErrors()
predictor.plotPredictions()
