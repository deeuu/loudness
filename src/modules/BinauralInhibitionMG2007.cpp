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
        Module("BinauralInhibitionMG2007")
    {
        LOUDNESS_DEBUG(name_ << ": Constructed.");
    }

    BinauralInhibitionMG2007::~BinauralInhibitionMG2007()
    {};

    bool BinauralInhibitionMG2007::initializeInternal(const SignalBank &input)
    {
        LOUDNESS_ASSERT(input.getNEars() == 2, name_
                << ": This module requires an input SignalBank with two ears.");

        /* Gaussian smoothing kernel. 
         * Since the kernel is symmetric, we just use one half.
         */
        Real camStep = input.getChannelSpacingInCams(); 
        gaussian_.assign (input.getNChannels(), 0.0);

        for (int chn = 0; chn < input.getNChannels(); ++chn)
        {
            Real g = camStep * chn;
            Real arg = (0.08 * g);
            gaussian_[chn] = exp(-arg * arg);
        }


        //output is same form as input
        output_.initialize (input);

        return 1;
    }

    void BinauralInhibitionMG2007::processInternal(const SignalBank &input)
    {       
        for (int src = 0; src < input.getNSources(); ++src)
        {
            const Real* inputSpecificLoudnessLeft = input
                                                    .getSingleSampleReadPointer
                                                    (src, 0, 0);
            const Real* inputSpecificLoudnessRight = input
                                                     .getSingleSampleReadPointer
                                                     (src, 1, 0);
            Real* outputSpecificLoudnessLeft = output_
                                               .getSingleSampleWritePointer
                                               (src, 0, 0);
            Real* outputSpecificLoudnessRight = output_
                                                .getSingleSampleWritePointer
                                                (src, 1, 0);

            int nChannels = input.getNChannels();
            for (int chn = 0; chn < nChannels; ++chn)
            { 
                /* Stage 1: Smooth the specific loudness patterns */
                Real smoothLeft = 0.0;
                Real smoothRight = 0.0;

                //Right side
                int i = chn, j = 0;
                while (i < nChannels)
                {
                    smoothLeft += inputSpecificLoudnessLeft[i] * gaussian_[j];
                    smoothRight += inputSpecificLoudnessRight[i++] * gaussian_[j++];
                }

                //left side
                j = nChannels - j;
                i = 0;
                while (j > 0)
                {
                    smoothLeft += inputSpecificLoudnessLeft[i] * gaussian_[j];
                    smoothRight += inputSpecificLoudnessRight[i++] * gaussian_[j--];
                }

                /* Stage 2: Inhibition using Eqs 2 and 3 */
                smoothLeft = max(smoothLeft, 1e-12);
                smoothRight = max(smoothRight, 1e-12);
                Real inhibLeft = 2 / (1 + pow(1.0 / cosh(smoothRight / smoothLeft), 1.5978));
                Real inhibRight = 2 / (1 + pow(1.0 / cosh(smoothLeft / smoothRight), 1.5978));

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
    }

    void BinauralInhibitionMG2007::resetInternal()
    {
    }
}
