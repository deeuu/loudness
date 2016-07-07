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

#include "ARAverager.h"
#include "../support/AuditoryTools.h"

namespace loudness{

    ARAverager::ARAverager(Real attackTime, Real releaseTime) :
        Module("ARAverager"),
        attackTime_(attackTime),
        releaseTime_(releaseTime)
    {
        LOUDNESS_DEBUG(name_ << ": Constructed.");
    }

    ARAverager::~ARAverager()
    {};

    bool ARAverager::initializeInternal(const SignalBank &input)
    {
        //filter coefficients
        attackCoef_ = 1 - exp(-1.0 / (input.getFrameRate() * attackTime_));
        releaseCoef_ = 1 - exp(-1.0 / (input.getFrameRate() * releaseTime_));

        LOUDNESS_DEBUG(name_ << ": Input frame rate: "
                << input.getFrameRate()
                << ". Attack time: "
                << attackTime_
                << ". Attack coefficient: "
                << attackCoef_
                << ". Release time: "
                << releaseTime_
                << ". Release coefficient: "
                << releaseCoef_);

        //output SignalBank
        output_.initialize(input);

        return 1;
    }

    void ARAverager::processInternal(const SignalBank &input)
    {       
        int lastSampleIdx = input.getNSamples() - 1;

        for (int src = 0; src < input.getNSources(); src++)
        {
            for (int ear = 0; ear < input.getNEars(); ++ear)
            {
                for (int chn = 0; chn < input.getNChannels(); ++chn)
                {
                    const Real* x = input.getSignalReadPointer(src,
                            ear, chn, 0);
                    Real* y = output_.getSignalWritePointer(src,
                            ear, chn, 0);
                    Real yPrev = y[lastSampleIdx];

                    for (int smp = 0; smp < input.getNSamples(); ++smp)
                    {
                        if (x[smp] > yPrev)
                            y[smp] = attackCoef_ * (x[smp] - yPrev) + yPrev;
                        else
                            y[smp] = releaseCoef_ * (x[smp] - yPrev) + yPrev;
                        yPrev = y[smp];
                    }
                }
            }
        }
    }

    //output SignalBanks are cleared so not to worry about filter state
    void ARAverager::resetInternal()
    {}
}
