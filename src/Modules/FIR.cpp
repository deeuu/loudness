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
            //constants
            order_ = (int)bCoefs_.size()-1;
            orderMinus1_ = order_-1;
            LOUDNESS_DEBUG("FIR: Filter order is: " << order_);

            //delay line
            z_.assign(order_,0.0);

            //output SignalBank
            output_.initialize(input.getNChannels(), input.getNSamples(), input.getFs());

            return 1;
        }
        {
            LOUDNESS_ERROR("FIR: No filter coefficients");
            return 0;
        }
    }

    void FIR::processInternal(const SignalBank &input)
    {
        int smp, j;
        Real x;

        LOUDNESS_DEBUG("FIR: New block");
        for(smp=0; smp<input.getNSamples(); smp++)
        {
            //input sample
            x = input.getSample(0, smp) * gain_;

            LOUDNESS_DEBUG("FIR: Input sample: " << x);

            //output sample
            output_.setSample(0, smp, bCoefs_[0] * x + z_[0]);

            //fill delay
            for (j=1; j<order_; j++)
                z_[j-1] = bCoefs_[j] * x + z_[j];

            //final sample
            z_[orderMinus1_] = bCoefs_[order_] * x;
        }
    }

    void FIR::resetInternal()
    {
        resetDelayLine();
    }
}


