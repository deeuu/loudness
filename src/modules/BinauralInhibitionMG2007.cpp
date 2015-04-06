/*
 * Copyright (C) 2014 Dominic Ward <contactdominicward@gmail.com>
 *
 * This file is part of Loudness
 *
 * Loudness is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Loudness is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Loudness.  If not, see <http://www.gnu.org/licenses/>. 
 */

#include "BinauralInhibitionMG2007.h"
#include "../support/AuditoryTools.h"

namespace loudness{

    BinauralInhibitionMG2007::BinauralInhibitionMG2007() :
        Module("BinauralInhibitionMG2007"),
    {
        LOUDNESS_DEBUG(name_ << ": Constructed.");
    }

    BinauralInhibitionMG2007::~BinauralInhibitionMG2007()
    {};

    bool BinauralInhibitionMG2007::initializeInternal(const SignalBank &input)
    {
        LOUDNESS_ASSERT(input.getNChannels() == 2, name_
                << ": This module requires an input SignalBank with two ears.");

        /* Smoothing kernel. Note:
         * We could save some memory and use a single kernel lookup table at the
         * cost of precision in the smoothed specific loudness patterns.
         */
        int nChannels = input.getNChannels();
        gaussians_.assign(nChannels, RealVec(nChannels, 0.0));

        for (int chn = 0; chn < nChannels; ++chn)
        {
            Real cfCam = freqToCam(input.getCentreFreq(chn));

            for (int chn2 = 0; chn2 < nChannels; ++chn2)
            {
                //normalised deviation from centre freq
                Real freqCam = freqToCam(input.getCentreFreq(chn2));
                Real g = (cfCam - fabs(cfCam - freqCam)) / cfCam;

                //the gaussians; 0.08 is tuning factor
                gaussians_[chn][chn2] = exp(-0.08 * g);
                gaussians_[chn][chn2] *= gaussians_[chn][chn2];
            }
        }

        //output is same form as input
        output_.initialize(input);

        return 1;
    }

    void BinauralInhibitionMG2007::processInternal(const SignalBank &input)
    {       
        const Real* inputSpecificLoudnessLeft = input.getSingleSampleReadPointer(0, 0);
        const Real* inputSpecificLoudnessRight = input.getSingleSampleReadPointer(1, 0);
        Real* outputSpecificLoudnessLeft = output_.getSingleSampleWritePointer(0, 0);
        Real* outputSpecificLoudnessRight = output_.getSingleSampleWritePointer(1, 0);

        int nChannels = input.getNChannels();
        for (int chn = 0; chn < nChannels; ++chn)
        {
            /* Stage 1: Smooth the specific loudness patterns */
            Real smoothLeft = 1e-12;
            Real smoothRight = 1e-12;

            for (int chn2 = 0; chn2 < nChannels; ++chn2)
            {
                smoothLeft += inputSpecificLoudnessLeft[chn2] * gaussians_[chn][chn2];
                smoothRight += inputSpecificLoudnessRight[chn2] * gaussians_[chn][chn2];
            }

            /* Stage 2: Inhibition using Eqs 2 and 3 */
            Real inhibLeft = 2 / (1 + pow(acosh(smoothRight / smoothLeft), 1.5978));
            Real inhibRight = 2 / (1 + pow(acosh(smoothRight / smoothLeft), 1.5978));

            /* Stage 3: Apply gains */
            outputSpecificLoudnessLeft[chn] = inputSpecificLoudnessLeft[chn] / inhibLeft;
            outputSpecificLoudnessRight[chn] = inputSpecificLoudnessRight[chn] / inhibRight;

            //to output smoothed specific loudness patterns, uncomment:
            /*
            outputSpecificLoudnessLeft[chn] = smoothLeft;
            outputSpecificLoudnessRight[chn] = smoothRight;
            */
        }
    }

    void BinauralInhibitionMG2007::resetInternal()
    {
    }
}
