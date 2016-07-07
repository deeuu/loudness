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

#include "PeakFollower.h"
#include "../support/AuditoryTools.h"

namespace loudness{

    PeakFollower::PeakFollower(Real timeConstant) 
        : Module("PeakFollower"),
          timeConstant_(timeConstant)
    {
        LOUDNESS_DEBUG(name_ << ": Constructed.");
    }

    PeakFollower::~PeakFollower()
    {};

    bool PeakFollower::initializeInternal(const SignalBank &input)
    {
        //filter coefficients
        coef_ = exp(-1.0 / (input.getFrameRate() * timeConstant_));

        LOUDNESS_DEBUG(name_ << ": Input frame rate: "
                << input.getFrameRate()
                << ". Release coefficient: "
                << coef_);

        //output SignalBank
        output_.initialize(input);

        return 1;
    }

    void PeakFollower::processInternal(const SignalBank &input)
    {       
        int lastSampleIdx = input.getNSamples() - 1;

        for (int ear = 0; ear < input.getNEars(); ++ear)
        {
            for (int chn = 0; chn < input.getNChannels(); ++chn)
            {
                const Real* x = input.getSignalReadPointer(ear, chn, 0);

                Real* y = output_.getSignalWritePointer(ear, chn, 0);

                Real yPrev = y[lastSampleIdx];

                for (int smp = 0; smp < input.getNSamples(); ++smp)
                {
                    Real absX = abs (x[smp]);

                    if (absX >= yPrev)
                        y[smp] = absX;
                    else
                        y[smp] = absX + coef_ * (yPrev - absX);
                    yPrev = y[smp];
                }
            }
        }
    }

    //output SignalBanks are cleared so not to worry about filter state
    void PeakFollower::resetInternal()
    {}
}
