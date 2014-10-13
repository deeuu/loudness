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

#include "Butter.h"

/*
 * Third order coefficients can be represented slightly better
 * see: http://www-users.cs.york.ac.uk/~fisher/mkfilter/trad.html
 * For 50Hz and 201Hz @32e3 Hz, coefficients give same result as in the link.
 */
namespace loudness{


    Butter::Butter(int order, int type, Real fc) :
        Module("Butter"),
        Filter(order),
        type_(type),
        fc_(fc)
    {}

    Butter::~Butter() {}

    bool Butter::initializeInternal(const SignalBank &input)
    {
        switch(order_)
        {
            case 3:
            {
                aCoefs_.resize(4);
                bCoefs_.resize(4);

                Real T = 1.0/input.getFs();
                Real wc = 2.0/T * tan(2*PI*fc_*T/2.0);
                Real c1 = T*T*wc*wc;
                Real c2 = c1*T*wc/8.0;
                Real c3 = T*wc;

                bCoefs_[0] = 1.0;
                bCoefs_[1] = -3.0;
                bCoefs_[2] = 3.0;
                bCoefs_[3] = -1.0;
                aCoefs_[0] = c2 + 0.5 * c1 + c3 + 1;
                aCoefs_[1] = 3*c2 + 0.5 * c1 - c3 - 3;
                aCoefs_[2] = 3*c2 - 0.5 * c1 - c3 + 3;
                aCoefs_[3] = c2 - 0.5 * c1 + c3 - 1;

                break;
            }
            default:
            {
                LOUDNESS_ERROR(name_ << ": Only third order low pass implemented.");
                return 0;
            }
        }

        //normalise by a[0]
        normaliseCoefs();

        //delay line
        z_.assign(2*order_,0.0);

        //output SignalBank
        output_.initialize(input.getNChannels(), input.getNSamples(), input.getFs());

        return 1;
    }

    void Butter::processInternal(const SignalBank &input)
    {
        switch(order_)
        {
            case 3:
                Real x,y;
                for(int smp=0; smp<input.getNSamples(); smp++)
                {
                    //input sample
                    x = input.getSample(0, smp) * gain_;
                    
                    //filter
                    y = bCoefs_[0]*(x-z_[2]) + bCoefs_[2]*(z_[1]-z_[0]) - 
                        aCoefs_[1]*z_[3] - aCoefs_[2]*z_[4] - aCoefs_[3]*z_[5];

                    //update delay line
                    z_[5] = z_[4];
                    z_[4] = z_[3];
                    z_[3] = y;
                    z_[2] = z_[1];
                    z_[1] = z_[0];
                    z_[0] = x;

                    //output sample
                    output_.setSample(0, smp, y);
                }
        }
    }
    void Butter::resetInternal()
    {
        resetDelayLine();
    }
}


