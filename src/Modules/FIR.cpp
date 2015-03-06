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
        int smp, j;
        int earIdx=0, chnIdx=0;
        Real x;

        for(int ear=0; ear<input.getNEars(); ear++)
        {
            for(int chn=0; chn<input.getNChannels(); chn++)
            {
                for(smp=0; smp<input.getNSamples(); smp++)
                {
                    //input sample
                    x = input.getSample(ear, chn, smp) * gain_;

                    //output sample
                    output_.setSample(ear, chn, smp, bCoefs_[earIdx][chnIdx][0] * x + z_[ear][chn][0]);

                    //fill delay
                    for (j=1; j<order_; j++)
                        z_[ear][chn][j-1] = bCoefs_[earIdx][chnIdx][j] * x + z_[ear][chn][j];

                    //final sample
                    z_[ear][chn][orderMinus1_] = bCoefs_[earIdx][chnIdx][order_] * x;
                }
            }
        }
    }

    void FIR::resetInternal()
    {
        resetDelayLine();
    }
}


