import loudness as ln
import numpy as np


def soneToPhon(x):
    return ln.soneToPhonMGB1997(float(x), True)


def measurementFunc(x, timeStep=0.004):
    start = int(0.6 / timeStep)
    end = int(2.0 / timeStep)
    return np.mean(x[start:end])

model = ln.DynamicLoudnessGM2002()
model.setHPFUsed(True)
model.setRate(1 / 0.004)
model.setFilterSpacingInCams(0.1)
fs = 32e3

predictor = ln.tools.predictors.DynamicLoudnessContourPredictor(
    model,
    fs,
    'LongTermLoudness',
    measurementFunc,
    soneToPhon,
    'abs'
)

predictor.alpha = 1.0
predictor.tol = 0.02
predictor.duration = 2.3
predictor.nIters = 20
predictor.targetLoudnessLevel = 2.2
predictor.process()
print predictor.computeErrors()
predictor.plotPredictions()
