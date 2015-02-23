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
        Module("Biquad"), Filter(2)
    {
        if(bCoefs.size()!=3) 
        { 
            LOUDNESS_WARNING(name_ << ": The order of this filter is 2." <<
                    "Length of feedforward coefficients is inappropriate." <<
                    "Continuing anyway, but you have been warned."); 
        }
        if(aCoefs.size()!=3)
        {
            LOUDNESS_WARNING(name_ << ": The order of this filter is 2." <<
                    "Length of feedback coefficients is inappropriate." <<
                    "Continuing anyway, but you have been warned."); 
        }
        setBCoefs(bCoefs);
        setACoefs(aCoefs);
    }

    Biquad::~Biquad() {}

    bool Biquad::initializeInternal(const SignalBank &input)
    {
        if((bCoefs_.size()==3) && (aCoefs_.size()==3))
        {
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

            //delay line
            z_.assign(2*order_,0.0);

            //output SignalBank
            output_.initialize(input.getNChannels(), input.getNSamples(), input.getFs());

            return 1;
        }
        else
        {
            LOUDNESS_ERROR(name_ << ": Inappropriate filter coefficients.");
            return 0;
        }
    }

    void Biquad::processInternal(const SignalBank &input)
    {
        Real x, y;
        for(int smp=0; smp<input.getNSamples(); smp++)
        {
            //input sample
            x = input.getSample(0, smp) * gain_;
            
            //filter
            y = bCoefs_[0]*x + bCoefs_[1]*z_[0] + bCoefs_[2]*z_[1] -
                aCoefs_[1]*z_[2] - aCoefs_[2]*z_[3];

            //update delay line
            z_[3] = z_[2];
            z_[2] = y;
            z_[1] = z_[0];
            z_[0] = x;

            //output sample
            output_.setSample(0, smp, y);
        }
    }

    void Biquad::setCoefficientFs(const Real coefficientFs)
    {
        coefficientFs_ = coefficientFs;
    }

    void Biquad::resetInternal()
    {
        resetDelayLine();
    }
}
