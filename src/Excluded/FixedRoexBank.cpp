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

#include "FixedRoexBank.h"
#include "../Support/AuditoryTools.h"

namespace loudness{

    FixedRoexBank::FixedRoexBank(Real freqLo, Real freqHi, Real camStep, Real level, bool spread) 
        : freqLo_(freqLo), freqHi_(freqHi), camStep_(camStep), level_(level), spread_(spread)
    {
    }

    FixedRoexBank::~FixedRoexBank()
    {
    }

    bool FixedRoexBank::initializeInternal(const SignalBank &input)
    {

        //lowest and highest fc of ERB filters
        const Real camHi = FreqToCam(freqHi_);
        const Real camLo = FreqToCam(freqLo_);
        LOUDNESS_DEBUG("FixedRoexBank: ERB lo: " << camLo << " ERB hi: " << camLo);

        //number of roex filters to use
        nFilters_ = round((camHi-camLo)/camStep_)+1; //+1 inclusive
        LOUDNESS_DEBUG("FixedRoexBank: Total number of filters: " << nFilters_);

        //initialize output SignalBank
        output_.initialize(nFilters_, 1, input.getFs());
        output_.setFrameRate(input.getFrameRate());
        LOUDNESS_DEBUG("FixedRoexBank: Output SignalBank frame rate: " << output_.getFrameRate() << " Hz");

        const Real p51_1k = 4000 / FreqToERB(1000.0);

        //filter shapes
        roex_.resize(nFilters_);

        //fill the above arrays
        Real cam, erb, fc, p, pu, t2, g, pg;
        for(int i=0; i<nFilters_; i++)
        {
            cam = camLo+(i*camStep_);
            fc = CamToFreq(cam);
            erb = FreqToERB(fc);
            pu = 4*fc/erb;
            t2 = 0.35*(pu/p51_1k);
            output_.setCentreFreq(i,fc);

            int j=0;
            while(j<input.getNChannels())
            {
                //normalised deviation
                g = (input.getCentreFreq(j)-fc)/fc;

                if (g<=2)
                {
                    if(g<0) //lower value 
                    {
                        p = pu-t2*(level_-51); 
                        pg = -p*g; //p*abs(g)
                    }
                    else //upper value
                    {
                        pg = pu*g;
                    }
                    //roex magnitude response
                    roex_[i].push_back((1+pg)*exp(-pg)); 
                }
                else
                    break;
                j++;
            }
        }

        LOUDNESS_DEBUG("DoubleRoexBank: Filters configured.");
        
        return 1;
    }


    void FixedRoexBank::processInternal(const SignalBank &input)
    {
        /*
         * Perform the excitation transformation
         */
        Real excitationLin;
        for(int i=0; i<nFilters_; i++)
        {
            excitationLin = 0;

            for(unsigned int j=0; j<roex_[i].size(); j++)
                excitationLin += roex_[i][j]*input.getSample(j,0);

            output_.setSample(i, 0, excitationLin);
        }

        /*  
        if(spread_)
        {
            Real p_prev = 0, step = 0;
            Real peak_dB, r_param, p_param, peak_dB_sqrd, alpha_param, out;
            bool got_peak = false;

            for(int e=0; e<nFilters_; e++)
            {
                excitationLin = output_.sample(e,0);

                //if input power is below smoothed power
                if (excitationLin<p_prev)
                {
                    //first entry so get the peak excitation
                    if(!got_peak)
                    {
                        peak_dB = 10*log10(p_prev);
                        got_peak=true;

                        //get the prediction coefficients
                        r_param =-1.7087606931878814*peak_dB + 227.89745866170813;
                        p_param = -0.04705140752229544*peak_dB + 6.79286774169383;
                        peak_dB_sqrd = peak_dB*peak_dB;
                        alpha_param = exp(2.5547449178119066e-06*peak_dB_sqrd*peak_dB + 
                            -0.0005008295771981608*peak_dB_sqrd + 0.046415398119506304*peak_dB + -5.076078085481516);
                        step = camStep_;
                    }

                    if(peak_dB>51) 
                    {
                        //predict slope
                        out = peak_dB+(r_param*(pow(1+p_param*step, alpha_param) * exp(-0.115*step))-r_param);

                        //output
                        output_.set_sample(e, 0, pow(10, out/10.0));

                        //output step
                        step += camStep_;
                    }
                }
                else
                    got_peak=false;
                p_prev = output_.sample(e,0);
            }
        }
        */
    }

    void FixedRoexBank::resetInternal(){};
}

