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

#include "SpecificPartialLoudnessCHGM2011.h"
#include "../support/AuditoryTools.h"

namespace loudness{

    SpecificPartialLoudnessCHGM2011::SpecificPartialLoudnessCHGM2011() :
        Module("SpecificPartialLoudnessCHGM2011")
    {
        LOUDNESS_DEBUG(name_ << ": Constructed.");
    }

    SpecificPartialLoudnessCHGM2011::~SpecificPartialLoudnessCHGM2011()
    {};

    bool SpecificPartialLoudnessCHGM2011::initializeInternal(const SignalBank &input)
    {
        for (int chn = 0; chn < input.getNChannels(); ++chn)
        {
            Real fc = input.getCentreFreq (chn);
            Real ef = hertzToNormalisedCam (fc);
            // Table 1 of Chen et al., 2011
            Real kDB = 6.51 * ef*ef - 1.93;
            LOUDNESS_DEBUG(fc << ": " << kDB);
            k_.push_back (std::pow (10, kDB / 10.0));
        }

        output_.initialize (input);

        return 1;
    }

    void SpecificPartialLoudnessCHGM2011::processInternal(const SignalBank &input)
    {       
        for (int ear = 0; ear < input.getNEars(); ++ear)
        {
            /*
             * Should probably calculate excitation patterns using total power
             * from all sources? They don't specify this in the paper...
             * Just go simple linear approach for now.
             */
            RealVec eTot (input.getNChannels(), 0.0);
            for (int src = 0; src < input.getNSources(); ++src)
            {
                const Real* inputExcitation = input
                                              .getSingleSampleReadPointer
                                              (src, ear, 0);

                for (int chn = 0; chn < input.getNChannels(); ++chn)
                    eTot[chn] += inputExcitation[chn];
            }

            // Now do partial loudness calculation
            for (int src = 0; src < input.getNSources(); ++src)
            {
                const Real* inputExcitation = input
                                              .getSingleSampleReadPointer
                                              (src, ear, 0);

                Real* outputExcitation = output_
                                         .getSingleSampleWritePointer
                                         (src, ear, 0);

                for (int chn = 0; chn < input.getNChannels(); ++chn)
                {
                    Real eNoise = eTot[chn] - inputExcitation[chn];
                    Real threshold = k_[chn] * eNoise;
                    if (inputExcitation[chn] > threshold)
                        outputExcitation[chn] = inputExcitation[chn] - threshold;
                    else
                        outputExcitation[chn] = 0.0;
                }
            }
        }
    }

    //output SignalBanks are cleared so not to worry about filter state
    void SpecificPartialLoudnessCHGM2011::resetInternal()
    {
    }
}
