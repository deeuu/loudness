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

#include "FixedRoexBank.h"
#include "../support/AuditoryTools.h"

namespace loudness{

    FixedRoexBank::FixedRoexBank(
            Real camLo,
            Real camHi,
            Real camStep,
            Real level) :
        Module("FixedRoexBank"),
        camLo_ (camLo),
        camHi_ (camHi),
        camStep_ (camStep),
        level_ (level)
    {}

    FixedRoexBank::~FixedRoexBank()
    {}

    bool FixedRoexBank::initializeInternal(const SignalBank &input)
    {

        int nFilters = std::floor((camHi_ - camLo_) / camStep_) + 1;

        LOUDNESS_DEBUG(name_ << ": Total number of filters: " << nFilters);

        //initialize output SignalBank
        output_.initialize (
                input.getNSources(),
                input.getNEars(),
                nFilters,
                1,
                input.getFs());
        output_.setFrameRate (input.getFrameRate());
        output_.setChannelSpacingInCams (camStep_);

        const Real p51_1k = 4000.0 / centreFreqToCambridgeERB(1000.0);

        //filter shapes
        roex_.resize(nFilters);

        //fill the above arrays
        Real cam, erb, fc, p, pu, t2, g, pg;
        for (int chn = 0; chn < nFilters; ++chn)
        {
            cam = camLo_ + camStep_ * chn;
            fc = camToHertz (cam);
            erb = centreFreqToCambridgeERB (fc);
            pu = 4.0 * fc / erb;
            t2 = 0.35 * (pu / p51_1k);
            output_.setCentreFreq (chn, fc);

            int j = 0;
            while (j < input.getNChannels())
            {
                //normalised deviation
                g = (input.getCentreFreq(j) - fc) / fc;

                if (g <= 2)
                {
                    if (g < 0) //lower value 
                    {
                        p = pu - t2 * (level_ - 51.0);
                        pg = -p * g; //p*abs(g)
                    }
                    else //upper value
                    {
                        pg = pu * g;
                    }

                    //roex magnitude response
                    roex_[chn].push_back ((1.0 + pg) * std::exp (-pg));
                }
                else
                    break;
                j++;
            }
        }

        return 1;
    }


    void FixedRoexBank::processInternal(const SignalBank &input)
    {
        for (int src = 0; src < input.getNSources(); ++src)
        {
            for (int ear = 0; ear < input.getNEars(); ++ear)
            {
                for (int chn = 0; chn < output_.getNChannels(); ++chn)
                {
                    const Real* inputSpectrum = input.
                                                getSingleSampleReadPointer 
                                                (src, ear, 0);
                    Real* outputExcitationPattern = output_.
                                                    getSingleSampleWritePointer
                                                    (src, ear, 0);
                    Real excitationLin = 0.0;

                    for (unsigned int j = 0; j < roex_[chn].size(); ++j)
                        excitationLin += roex_[chn][j] * inputSpectrum[j];

                    outputExcitationPattern[chn] = excitationLin;
                }
            }
        }
    }

    void FixedRoexBank::resetInternal(){};
}

