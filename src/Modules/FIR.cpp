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
        if(bCoefs_.size()>0)
        {
            //constants, order currently fixed for all ears/channels
            order_ = (int)bCoefs_.size()-1;
            orderMinus1_ = order_-1;
            LOUDNESS_DEBUG("FIR: Filter order is: " << order_);

            //delay line
            z_.assign(input.getNEars() * order_, 0.0);

            //output SignalBank
            output_.initialize(input);

            return 1;
        }
        {
            LOUDNESS_ERROR("FIR: No filter coefficients");
            return 0;
        }
    }

    void FIR::processInternal(const SignalBank &input)
    {

        for(int ear=0; ear<input.getNEars(); ear++)
        {
            int smp, j;
            int zIdx = ear * order_;
            const Real *inputSignal = input.getSignal(ear, 0);
            Real *outputSignal = output_.getSignal(ear, 0);
            Real x;

            for(smp=0; smp<input.getNSamples(); smp++)
            {
                //input sample
                x = inputSignal[smp] * gain_;

                //output sample
                outputSignal[smp] = bCoefs_[0] * x + z_[zIdx];

                //fill delay
                for (j=1; j<order_; j++)
                    z_[zIdx+j-1] = bCoefs_[j] * x + z_[zIdx+j];

                //final sample
                z_[zIdx + orderMinus1_] = bCoefs_[order_] * x;
            }
        }
    }

    void FIR::resetInternal()
    {
        resetDelayLine();
    }
}
