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
        uint n_b = bCoefs_[0][0].size();
        uint n_a = aCoefs_[0][0].size();
        if(n_b*n_a > 0)
        {
            if(n_b > n_a)
            {
                for(uint ear=0; ear<aCoefs_.size(); ear++)
                {
                    for(uint chn=0; chn<aCoefs_[ear].size(); chn++)
                        aCoefs_[ear][chn].resize(n_b,0);
                }
                order_ = n_b-1;
            }
            else
            {
                for(uint ear=0; ear<bCoefs_.size(); ear++)
                {
                    for(uint chn=0; chn<bCoefs_[ear].size(); chn++)
                        bCoefs_[ear][chn].resize(n_b,0);
                }
                order_ = n_a-1;
            }

            LOUDNESS_DEBUG("IIR: Filter order is: " << order_);
            orderMinus1_ = order_-1;

            //Normalise coefficients if a[0] != 1
            normaliseCoefs();

            if(!checkBCoefs(input))
                return 0;

            //delay line
            z_.assign(input.getNEars(),
                    RealVecVec(input.getNChannels(), 
                        RealVec(order_, 0.0)));

            //output SignalBank
            output_.initialize(input);

            return 1;
        }
        {
            LOUDNESS_ERROR(name_ << ": No filter coefficients");
            return 0;
        }
    }

    void IIR::processInternal(const SignalBank &input)
    {
        int smp, j;
        int earIdx=0, chnIdx=0;
        Real x, y;

        for(int ear=0; ear<input.getNEars(); ear++)
        {
            if (!duplicateEarCoefs_)
                earIdx=ear;
            for(int chn=0; chn<input.getNChannels(); chn++)
            {
                for(smp=0; smp<input.getNSamples(); smp++)
                {
                    //input sample
                    x = input.getSample(ear, chn, smp) * gain_;

                    //output sample
                    y = bCoefs_[earIdx][chnIdx][0] * x + z_[ear][chn][0];
                    output_.setSample(ear, chn, smp, y);

                    //fill delay
                    for (j=1; j<order_; j++)
                        z_[ear][chn][j-1] = bCoefs_[earIdx][chnIdx][j] * x;
                    z_[ear][chn][orderMinus1_] = bCoefs_[earIdx][chnIdx][order_] * x;
                    for (j=1; j<order_; j++)
                        z_[ear][chn][j-1] += z_[ear][chn][j];
                    for (j=1; j<order_; j++)
                        z_[ear][chn][j-1] -= aCoefs_[earIdx][chnIdx][j] * y;
                    z_[ear][chn][orderMinus1_] -= aCoefs_[earIdx][chnIdx][order_] * y;

                }
            }
        }
    }

    void IIR::resetInternal()
    {
        resetDelayLine();
    }

}


