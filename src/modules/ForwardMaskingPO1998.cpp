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

#include "ForwardMaskingPO1998.h"

namespace loudness{

    ForwardMaskingPO1998::ForwardMaskingPO1998 (Real timeConstant1,
                                                Real timeConstant2,
                                                Real weight) :
        Module ("ForwardMaskingPO1998"),
        Filter (2),
        timeConstant1_ (timeConstant1),
        timeConstant2_ (timeConstant2),
        weight_ (weight)
    {
        LOUDNESS_DEBUG(name_ << ": Constructed.");
    }

    ForwardMaskingPO1998::~ForwardMaskingPO1998()
    {}

    bool ForwardMaskingPO1998::initializeInternal(const SignalBank &input)
    {
        Real weight2 = 1 - weight_;
        Real alpha1 = std::exp (-1.0 / (timeConstant1_ * input.getFrameRate()));
        Real alpha2 = std::exp (-1.0 / (timeConstant2_ * input.getFrameRate()));

        // coefs for  n-1
        RealVec bCoefs = {-(weight2 * alpha2 + weight_ * alpha1)};
        // coefs for n, n-1 and n-2
        RealVec aCoefs = {1.0, -alpha1 - alpha2, alpha1 * alpha2};

        aCoefs[0] = 1.0 / ((1 - weight_) * timeConstant1_ + 
                    weight_ * timeConstant2_);
        aCoefs[0] /= input.getFrameRate();

        setBCoefs (bCoefs);
        setACoefs (aCoefs);

        delayLine_.initialize (
                input.getNSources(),
                input.getNEars(),
                input.getNChannels(),
                2,
                input.getFs());
                    

        //output SignalBank
        output_.initialize(input);

        return 1;
    }

    void ForwardMaskingPO1998::processInternal(const SignalBank &input)
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
                        //input sample
                        Real x = inputSignal[smp];

                        //output sample
                        outputSignal[smp] = x + z[0];

                        // b[n-1] ...
                        z[0] = bCoefs_[0] * x + z[1];
                        z[1] = 0.0;

                        // a[n-1] ...
                        z[0] -= aCoefs_[1] * outputSignal[smp];

                        // a[n-2] ...
                        z[1] -= aCoefs_[2] * outputSignal[smp];
                        
                        outputSignal[smp] *= aCoefs_[0];
                            
                    }

                    killDenormal (z[0]);
                    killDenormal (z[1]);
                }
            }
        }
    }

    //output SignalBanks are cleared so not to worry about filter state
    void ForwardMaskingPO1998::resetInternal()
    {}
}
