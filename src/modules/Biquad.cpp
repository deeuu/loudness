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

#include "Biquad.h"

namespace loudness{

    Biquad::Biquad() :
        Module("Biquad"),
        Filter(2)
    {}
    
    Biquad::Biquad(const RealVec &bCoefs, const RealVec &aCoefs) :
        Module("Biquad"),
        Filter(2)
    {
        setBCoefs(bCoefs);
        setACoefs(aCoefs);
    }

    Biquad::~Biquad() {}

    bool Biquad::initializeInternal(const SignalBank &input)
    {
        LOUDNESS_ASSERT( bCoefs_.size() == 3 &&
                aCoefs_.size() == 3,
                "Filter coefficients do not satisfy filter order");

        //normalise by a[0]
        normaliseCoefs();

        //Transform coefficients if sampling frequency is different from
        //the one used in origin filter design
        //See: Parameter Quantization in Direct-Form Recursive Audio Filters
        //by Neunaber (2008)
        if((coefficientFs_ != 0) && (coefficientFs_ != input.getFs()))
        {
            double fc = (coefficientFs_/PI) * atan( sqrt( (1+aCoefs_[1]+aCoefs_[2]) /
                        (1-aCoefs_[1]+aCoefs_[2])));
            double Q = sqrt((aCoefs_[2]+1)*(aCoefs_[2]+1) - aCoefs_[1]*aCoefs_[1]) /
                        (2*fabs(1-aCoefs_[2]));
            double Vl = (bCoefs_[0]+bCoefs_[1]+bCoefs_[2]) / (1+aCoefs_[1]+aCoefs_[2]);
            double Vb = (bCoefs_[0] - bCoefs_[2]) / (1-aCoefs_[2]);
            double Vh = (bCoefs_[0]-bCoefs_[1]+bCoefs_[2]) / (1-aCoefs_[1]+aCoefs_[2]);

            double omega = tan(PI*fc/input.getFs());
            double omegaSqrd = omega*omega;
            double denom = omegaSqrd + omega/Q + 1;

            aCoefs_[0] = 1.0;
            aCoefs_[1] = 2*(omegaSqrd - 1) / denom;
            aCoefs_[2] = (omegaSqrd - (omega/Q) + 1)/ denom;

            bCoefs_[0] = (Vl * omegaSqrd + Vb * (omega/Q) + Vh) / denom;
            bCoefs_[1] = 2*(Vl*omegaSqrd - Vh) / denom;
            bCoefs_[2] = (Vl*omegaSqrd - (Vb*omega/Q) + Vh) / denom;
        }

        delayLine_.initialize(input.getNSources(),
                              input.getNEars(),
                              input.getNChannels(),
                              2 * order_,
                              input.getFs());

        //output SignalBank
        output_.initialize(input);

        return 1;
    }

    void Biquad::processInternal(const SignalBank &input)
    {
        for (int src = 0; src < input.getNSources(); ++src)
        {
            for (int ear = 0; ear < input.getNEars(); ++ear)
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
                        Real x = inputSignal[smp] * gain_;
                        
                        //filter
                        Real y = bCoefs_[0]*x + bCoefs_[1]*z[0] + bCoefs_[2]*z[1]
                                 - aCoefs_[1]*z[2] - aCoefs_[2]*z[3];

                        //update delay line
                        z[3] = z[2];
                        z[2] = y;
                        z[1] = z[0];
                        z[0] = x;

                        //output sample
                        outputSignal[smp] = y;
                    }
                }
            }
        }
    }

    void Biquad::setCoefficientFs(const Real coefficientFs)
    {
        coefficientFs_ = coefficientFs;
    }

    void Biquad::resetInternal()
    {
        delayLine_.zeroSignals();
    }
}
