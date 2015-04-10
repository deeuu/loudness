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

#include "DoubleRoexBank.h"
#include "../support/AuditoryTools.h"

namespace loudness{

    DoubleRoexBank::DoubleRoexBank(Real camLo, 
            Real camHi,
            Real camStep,
            Real scalingFactor) : 
        Module("DoubleRoexBank"),
        camLo_(camLo), 
        camHi_(camHi),
        camStep_(camStep),
        scalingFactor_(scalingFactor)
    {}

    DoubleRoexBank::~DoubleRoexBank() {}

    bool DoubleRoexBank::initializeInternal(const SignalBank &input)
    {

        //number of roex filters to use
        nFilters_ = round((camHi_-camLo_)/camStep_)+1; //+1 inclusive
        LOUDNESS_DEBUG(name_ << ": Total number of filters: " << nFilters_);

        //initialize output SignalBank
        output_.initialize(input.getNEars(), nFilters_, 1, input.getFs());
        output_.setFrameRate(input.getFrameRate());
        output_.setChannelSpacingInCams(camStep_);

        //filter variables
        wPassive_.resize(nFilters_);
        wActive_.resize(nFilters_);
        maxGdB_.resize(nFilters_);
        thirdGainTerm_.resize(nFilters_);

        //fill the above arrays
        Real cam, fc, tl, tu, pl, pu, g, pgPassive, pgActive;
        for (int i = 0; i < nFilters_; i++)
        {
            //erb to frequency
            cam = camLo_+(i*camStep_);
            fc = camToFreq(cam);
            output_.setCentreFreq(i,fc);

            //slopes
            tl = fc/(0.108*fc+2.33);
            tu = 15.6;
            pl = fc/(0.027*fc+5.44);
            pu = 27.9;

            //precalculate some gain terms
            maxGdB_[i] = fc/(0.0191*fc+1.1);
            thirdGainTerm_[i] = maxGdB_[i]/(1+exp(0.05*(100-maxGdB_[i])));

            //compute the fixed filters
            int j=0;
            while (j < input.getNChannels())
            {
                //normalised deviation
                g = (input.getCentreFreq(j)-fc)/fc;

                //Is g limited to 2 sufficient for the passive filter?
                if (g <= 2)
                {
                    if (g < 0) //lower value 
                    {
                        pgPassive = -tl*g; 
                        pgActive = -pl*g; 
                    }
                    else //upper value
                    {
                        pgPassive = tu*g;
                        pgActive = pu*g; 
                    }

                    wPassive_[i].push_back((1+pgPassive)*exp(-pgPassive)); 
                    wActive_[i].push_back((1+pgActive)*exp(-pgActive)); 
                }
                else
                    break;
                j++;
            }
        }
        LOUDNESS_DEBUG(name_ << ": Passive and active filters configured.");
        LOUDNESS_DEBUG(name_ << ": Excitation pattern will be scaled by: " 
                << scalingFactor_);
        
        return 1;
    }


    void DoubleRoexBank::processInternal(const SignalBank &input)
    {
        /*
         * Perform the excitation transformation
         */
        Real excitationLinP, excitationLinA;
        Real excitationLog, gain, excitationLogMinus30;
        for (int ear = 0; ear < input.getNEars(); ear++)
        {
            const Real* inputSpectrum = input.getSingleSampleReadPointer(ear, 0);
            Real* outputExcitationPattern = output_.getSingleSampleWritePointer(ear, 0);

            for (int i = 0; i < nFilters_; i++)
            {
                excitationLinP = 0.0;
                excitationLinA = 0.0;

                //passive filter output
                for (uint j = 0; j < wPassive_[i].size(); j++)
                    excitationLinP += wPassive_[i][j] * inputSpectrum[j];

                //convert to dB
                excitationLog = 10*log10(excitationLinP+LOW_LIMIT_POWER);

                //compute gain
                gain = maxGdB_[i] - (maxGdB_[i] / (1 + exp(-0.05*(excitationLog-(100-maxGdB_[i]))))) 
                    + thirdGainTerm_[i];

                //check for higher levels
                if (excitationLog > 30)
                {
                    excitationLogMinus30 = excitationLog-30;
                    gain = gain - 0.003*excitationLogMinus30*excitationLogMinus30;
                }

                //convert to linear gain
                gain = pow(10, gain/10.0); 

                //active filter output
                for (uint j = 0; j < wActive_[i].size(); j++)
                    excitationLinA += wActive_[i][j] * inputSpectrum[j];
                excitationLinA *= gain;

                //excitation pattern
                outputExcitationPattern[i] = scalingFactor_ * (excitationLinP + excitationLinA);
            }
        }
    }

    void DoubleRoexBank::resetInternal(){};
}
