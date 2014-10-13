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

#include "IIR.h"

namespace loudness{

    IIR::IIR() : Module("IIR") {};

    IIR::IIR(const RealVec &bCoefs, const RealVec &aCoefs) :
        Module("IIR")
    {
        setBCoefs(bCoefs);
        setACoefs(aCoefs);
    }

    IIR::~IIR()
    {
    }

    bool IIR::initializeInternal(const SignalBank &input)
    {

        //constants
        int n_b = (int)bCoefs_.size();
        int n_a = (int)aCoefs_.size();
        if(n_b*n_a > 0)
        {
            if(n_b > n_a)
            {
                aCoefs_.resize(n_b,0);
                order_ = n_b-1;
            }
            else
            {
                bCoefs_.resize(n_a,0);
                order_ = n_a-1;
            }
            LOUDNESS_DEBUG("IIR: Filter order is: " << order_);
            orderMinus1_ = order_-1;

            //Normalise coefficients if a[0] != 1
            normaliseCoefs();

            //delay line
            z_.assign(order_,0.0);

            //output SignalBank
            output_.initialize(input.getNChannels(), input.getNSamples(), input.getFs());

            return 1;
        }
        {
            LOUDNESS_ERROR("IIR: No filter coefficients");
            return 0;
        }
    }

    void IIR::processInternal(const SignalBank &input)
    {
        int smp, j;
        Real x,y;

        for(smp=0; smp<input.getNSamples(); smp++)
        {
            //input sample
            x = input.getSample(0, smp) * gain_;

            //output sample
            y = bCoefs_[0] * x + z_[0];
            output_.setSample(0, smp, y);

            //fill delay
            for (j=1; j<order_; j++)
                z_[j-1] = bCoefs_[j] * x + z_[j] - aCoefs_[j] * y;

            //final sample
            z_[orderMinus1_] = bCoefs_[order_] * x - aCoefs_[order_] * y;
        }
    }

    void IIR::resetInternal()
    {
        resetDelayLine();
    }

}


