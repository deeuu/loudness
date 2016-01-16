import numpy as np
import loudness as ln


class BinauralInhibitionMG2007:

    def __init__(self, cams):

        nFilters = cams.size
        camStep = cams[1] - cams[0]
        g = np.arange(nFilters) * camStep
        gaussian = np.exp(-(0.08 * g) ** 2)

        self.fullGaussian = np.hstack((gaussian[::-1], gaussian[1:]))

    def process(self, specificLoudness):

        # convolution with weighting function
        self.smoothSpecificLoudness = np.zeros(specificLoudness.shape)
        nFilters = specificLoudness.shape[0]
        for ear in range(2):
            self.smoothSpecificLoudness[:, ear] = \
                np.convolve(
                    self.fullGaussian, specificLoudness[:, ear]
                )[nFilters-1:-nFilters+1]

        # Gain computation
        self.smoothSpecificLoudness[self.smoothSpecificLoudness < 1e-12] = \
            1e-12
        inhibLeft = \
            2.0 / (1 + (1.0 / np.cosh(
                self.smoothSpecificLoudness[:, 1] /
                self.smoothSpecificLoudness[:, 0]
            )) ** 1.5978)
        inhibRight = 2.0 / (1 + (1.0 / np.cosh(
            self.smoothSpecificLoudness[:, 0] /
            self.smoothSpecificLoudness[:, 1]
        )) ** 1.5978)

        # Apply gains to original specific loudness
        self.inhibitedSpecificLoudness = specificLoudness.copy()
        self.inhibitedSpecificLoudness[:, 0] /= inhibLeft
        self.inhibitedSpecificLoudness[:, 1] /= inhibRight

# python side
cams = np.arange(1.8, 39, 0.1)
model = BinauralInhibitionMG2007(cams)

# input specific loudness patterns (one per ear)
pattern = np.random.rand(cams.size, 2)
model.process(pattern)

# loudness side
specificLoudnessBank = ln.SignalBank()
specificLoudnessBank.initialize(2, cams.size, 1, 1)
specificLoudnessBank.setChannelSpacingInCams(0.1)
specificLoudnessBank.setSignals(pattern.T.reshape((2, pattern.shape[0], 1)))

binauralModel = ln.BinauralInhibitionMG2007()
binauralModel.initialize(specificLoudnessBank)
binauralModel.process(specificLoudnessBank)

inhibitedSpecificLoudnessBank = binauralModel.getOutput()
inhibitedSpecificLoudness =\
    inhibitedSpecificLoudnessBank.getSignals().reshape((2, cams.size)).T

print ("Python vs loudness equality test applied to "
       + "inhibited specific loudness patterns: %r "
       % np.allclose(inhibitedSpecificLoudness,
                     model.inhibitedSpecificLoudness))
