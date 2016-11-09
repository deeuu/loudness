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

#include "MultiSourceMasking3.h"
#include "../support/AuditoryTools.h"

namespace loudness{

    MultiSourceMasking3::MultiSourceMasking3(Real offsetKDB) :
        Module("MultiSourceMasking3"),
        offsetKDB_ (offsetKDB)
    {
        LOUDNESS_DEBUG(name_ << ": Constructed.");
    }

    MultiSourceMasking3::~MultiSourceMasking3()
    {};

    bool MultiSourceMasking3::initializeInternal(const SignalBank &input)
    {

        k_.resize (input.getNChannels());
        Real temp = decibelsToPower (-std::abs(offsetKDB_));
        for (int chn = 0; chn < input.getNChannels(); ++chn)
        {
            Real freq = input.getCentreFreq (chn);

            if (freq>=500)
            {
                k_[chn] = temp * 3.73;
            }
            else
            {
                Real logFreq;
                if(freq<=50)
                    logFreq = log10(50);
                else
                    logFreq = log10(freq);

                k_[chn] = (temp * std::pow(10, (0.43068810954936998 * pow(logFreq,3) 
                              - 2.7976098820730675 * pow(logFreq,2)
                              + 5.0738460335696969 * logFreq -
                              1.2060617476790148)));
            }
            k_[chn] *= temp;
        }


        output_.initialize (input);

        return 1;
    }

    void MultiSourceMasking3::processInternal(const SignalBank &input)
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
    void MultiSourceMasking3::resetInternal()
    {
    }
}
