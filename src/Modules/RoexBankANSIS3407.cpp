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

#include "RoexBankANSIS3407.h"
#include "../Support/AuditoryTools.h"

namespace loudness{

    RoexBankANSIS3407::RoexBankANSIS3407(Real camLo, Real camHi, Real camStep) : 
        Module("RoexBankANSIS3407"),
        camLo_(camLo), 
        camHi_(camHi),
        camStep_(camStep)
    {}

    RoexBankANSIS3407::~RoexBankANSIS3407()
    {
    }

    bool RoexBankANSIS3407::initializeInternal(const SignalBank &input)
    {
        //number of input components
        int nChannels = input.getNChannels();

        //number of roex filters to use
        nFilters_ = round((camHi_-camLo_)/camStep_)+1; //+1 inclusive

        //initialize output SignalBank
        output_.initialize(nFilters_, 1, input.getFs());
        output_.setFrameRate(input.getFrameRate());

        const Real p51_1k = 4000 / FreqToERB(1000.0);

        //pcomp is slope per component
        pcomp_.assign(nChannels, 0.0);

        //comp_level holds level per ERB on each component
        compLevel_.assign(nChannels, 0.0);

        //p upper is level invariant
        pu_.assign(nFilters_, 0.0);
        
        //p lower is level dependent
        pl_.assign(nFilters_, 0.0);

        //fill the above arrays
        Real cam, erb, fc;
        for(int i=0; i<nChannels; i++)
        {
            fc = input.getCentreFreq(i);
            erb = FreqToERB(fc);
            pcomp_[i] = 4*fc/erb;
        }
        for(int i=0; i<nFilters_; i++)
        {
            cam = camLo_+(i*camStep_);
            fc = CamToFreq(cam);
            erb = FreqToERB(fc);
            pu_[i] = 4*fc/erb;
            pl_[i] = 0.35*(pu_[i]/p51_1k);
            output_.setCentreFreq(i,fc);
        }

        return 1;
    }


    void RoexBankANSIS3407::processInternal(const SignalBank &input)
    {
        Real excitationLin, fc, g, p, pg;

        //ANSI 2007 style: calculate level per ERB
        //using level independent roex filters centred on every component
        int j;
        int nChannels = input.getNChannels();
        for(int i=0; i<nChannels; i++)
        {
            excitationLin = 0;
            j=0;
            fc = input.getCentreFreq(i);

            while(j<nChannels)
            {
                //normalised deviation
                g = (input.getCentreFreq(j)-fc)/fc;

                if(g>2)
                    break;
                if(g<0) //lower value 
                    pg = -pcomp_[i]*g; //p*abs(g)
                else //upper value
                    pg = pcomp_[i]*g;
                //excitation per erb
                excitationLin += (1+pg)*exp(-pg)*input.getSample(j++,0); 
            }

            //convert to dB, subtract 51 here to save operations later
            if (excitationLin < 1e-10)
                compLevel_[i] = -151.0;
            else
                compLevel_[i] = 10*log10(excitationLin)-51;

            LOUDNESS_DEBUG("RoexBankGM: ERB/dB : " << compLevel_[i]);
        }
        
        //now the excitation pattern
        for(int i=0; i<nFilters_; i++)
        {
            excitationLin = 0;
            j=0;
            fc = output_.getCentreFreq(i);

            while(j<nChannels)
            {
                //normalised deviation
                g = (input.getCentreFreq(j)-fc)/fc;

                if (g>2)
                    break;
                if(g<0) //lower value 
                {
                    //checked out 2.4.14
                    p = pu_[i]-(pl_[i]*compLevel_[j]); //51dB subtracted above
                    p = p < 0.1 ? 0.1 : p; //p can go negative for very high levels
                    pg = -p*g; //p*abs(g)
                }
                else //upper value
                {
                    pg = pu_[i]*g;
                }

                excitationLin += (1+pg)*exp(-pg)*input.getSample(j++,0); //excitation per erb
            }

            output_.setSample(i, 0, excitationLin);
        }
    }

    void RoexBankANSIS3407::resetInternal(){};
}

