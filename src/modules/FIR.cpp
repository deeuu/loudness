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

#include "FIR.h"

namespace loudness{

    FIR::FIR() : Module("FIR") {}

    FIR::FIR(const RealVec &bCoefs) :
        Module("FIR")
    {
        setBCoefs(bCoefs);
    }

    FIR::~FIR() {}

    bool FIR::initializeInternal(const SignalBank &input)
    {
        LOUDNESS_ASSERT(bCoefs_.size() > 0, name_ << ": No filter coefficients");

        //constants, order currently fixed for all ears/channels
        order_ = (int)bCoefs_.size() - 1;
        orderMinus1_ = order_ - 1;
        LOUDNESS_DEBUG("FIR: Filter order is: " << order_);

        //internal delay line - single vector for all ears
        delayLine_.initialize (input.getNSources(),
                   input.getNEars(),
                   input.getNChannels(),
                   order_,
                   input.getFs());

        //output SignalBank
        output_.initialize (input);

        return 1;
    }

    void FIR::processInternal(const SignalBank &input)
    {
        for (int src = 0; src < input.getNSources(); ++src)
        {
            for(int ear = 0; ear < input.getNEars(); ++ear)
            {
                for (int chn = 0; chn < input.getNChannels(); ++chn)
                {
                    const Real* inputSignal = input.getSignalReadPointer
                                                    (src, ear, chn);
                    Real* outputSignal = output_.getSignalWritePointer
                                                 (src, ear, chn);
                    Real* z = delayLine_.getSignalWritePointer
                                         (src, ear, chn);

                    for (int smp = 0; smp < input.getNSamples(); ++smp)
                    {
                        Real x = inputSignal[smp];

                        //output sample
                        outputSignal[smp] = bCoefs_[0] * x + z[0];

                        //fill delay
                        for (int j = 1; j < order_; ++j)
                            z[j-1] = bCoefs_[j] * x + z[j];

                        //final sample
                        z[orderMinus1_] = bCoefs_[order_] * x;
                    }
                }
            }
        }
    }

    void FIR::resetInternal()
    {
        delayLine_.zeroSignals();
    }
}
