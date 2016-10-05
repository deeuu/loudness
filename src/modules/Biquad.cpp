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

    Biquad::Biquad(const string &type) :
        Module("Biquad"),
        Filter(2),
        type_ (type)
    {}

    Biquad::~Biquad() {}

    bool Biquad::initializeInternal(const SignalBank &input)
    {
        if (type_ == "RLB")
        {
            RealVec bCoefs = {1.0, -2.0, 1.0};
            RealVec aCoefs = {1.0, -1.99004745483398, 0.99007225036621};
            setBCoefs (bCoefs);
            setACoefs (aCoefs);
            setCoefficientFs (48000);
        }
        else if (type_ == "prefilter")
        {
            RealVec bCoefs = {1.53512485958697,
                              -2.69169618940638, 
                              1.19839281085285};
            RealVec aCoefs = {1.0,
                              -1.69065929318241, 
                              0.73248077421585};
            setBCoefs (bCoefs);
            setACoefs (aCoefs);
            setCoefficientFs (48000);
        }

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

        /*
        std::cout << "A" << std::endl;
        for (int i = 0; i < 3; ++i)
            std::cout << aCoefs_[i] << std::endl;
        std::cout << "B" << std::endl;
        for (int i = 0; i < 3; ++i)
            std::cout << bCoefs_[i] << std::endl;
        */

        delayLine_.initialize(input.getNSources(),
                              input.getNEars(),
                              input.getNChannels(),
                              2,
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
                        //filter
                        Real x = inputSignal[smp];
                        Real y = bCoefs_[0] * x + z[0];
                        z[0] = bCoefs_[1] * x - aCoefs_[1] * y + z[1];
                        z[1] = bCoefs_[2] * x - aCoefs_[2] * y;
                        outputSignal[smp] = y;
                    }

                    killDenormal (z[0]);
                    killDenormal (z[1]);
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
