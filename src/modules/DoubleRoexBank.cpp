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
            Real scalingFactor,
            bool isExcitationPatternInterpolated,
            bool isInterpolationCubic) : 
        Module("DoubleRoexBank"),
        camLo_(camLo), 
        camHi_(camHi),
        camStep_(camStep),
        scalingFactor_(scalingFactor),
        isExcitationPatternInterpolated_(isExcitationPatternInterpolated),
        isInterpolationCubic_(isInterpolationCubic)
    {}

    DoubleRoexBank::~DoubleRoexBank() {}

    bool DoubleRoexBank::initializeInternal(const SignalBank &input)
    {

        if (camStep_ <= 0.1)
            isExcitationPatternInterpolated_ = false;
        if (isExcitationPatternInterpolated_)
        {
            camLo_ = 1.5;
            camHi_ = 40.2;
        }

        //number of roex filters to use
        nFilters_ = round((camHi_-camLo_)/camStep_)+1; //+1 inclusive

        LOUDNESS_DEBUG(name_
                << ": interpolation applied: " << isExcitationPatternInterpolated_
                << " filter spacing in Cams: " << camStep_
                << " Total number of filters: " << nFilters_);

        //initialise output SignalBank
        if(isExcitationPatternInterpolated_)
        {
            //centre freqs in cams
            cams_.assign(nFilters_, 0);

            //required for log interpolation
            logExcitation_.assign(nFilters_, 0.0);

            //388 filters to cover [1.5, 40.2]
            output_.initialize(input.getNEars(), 388, 1, input.getFs());
            output_.setChannelSpacingInCams(0.1);
            for (int i = 0; i < 388; ++i)
                output_.setCentreFreq(i, camToFreq(camLo_ + (i * 0.1)));
        }
        else
        {
            output_.initialize(input.getNEars(), nFilters_, 1, input.getFs());
            output_.setChannelSpacingInCams(camStep_);
        }

        output_.setFrameRate(input.getFrameRate());

        //filter variables
        wPassive_.resize(nFilters_);
        wActive_.resize(nFilters_);
        maxGdB_.resize(nFilters_);
        thirdGainTerm_.resize(nFilters_);

        //fill the above arrays
        for (int i = 0; i < nFilters_; i++)
        {
            Real cam = camLo_ + (i * camStep_);
            Real fc = camToFreq(cam);

            if (isExcitationPatternInterpolated_)
                cams_[i] = cam;
            else
                output_.setCentreFreq(i, fc);

            //slopes
            Real tl = fc/(0.108*fc+2.33);
            Real tu = 15.6;
            Real pl = fc/(0.027*fc+5.44);
            Real pu = 27.9;

            //precalculate some gain terms
            maxGdB_[i] = fc/(0.0191*fc+1.1);
            thirdGainTerm_[i] = maxGdB_[i]/(1+exp(0.05*(100-maxGdB_[i])));

            //compute the fixed filters
            int j = 0;
            while (j < input.getNChannels())
            {
                Real pgPassive, pgActive;

                //normalised deviation
                Real g = (input.getCentreFreq(j)-fc)/fc;

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
        for (int ear = 0; ear < input.getNEars(); ++ear)
        {
            const Real* inputSpectrum = input.getSingleSampleReadPointer(ear, 0);
            Real* outputExcitationPattern = output_.getSingleSampleWritePointer(ear, 0);

            for (int i = 0; i < nFilters_; ++i)
            {
                excitationLinP = 0.0;
                excitationLinA = 0.0;

                //passive filter output
                for (uint j = 0; j < wPassive_[i].size(); j++)
                    excitationLinP += wPassive_[i][j] * inputSpectrum[j];

                //convert to dB
                excitationLog = powerToDecibels(excitationLinP);

                //compute gain
                gain = maxGdB_[i] - (maxGdB_[i] / (1 + exp(-0.05*(excitationLog-(100-maxGdB_[i]))))) 
                    + thirdGainTerm_[i];

                //check for higher levels
                if (excitationLog > 30)
                {
                    excitationLogMinus30 = excitationLog - 30;
                    gain = gain - 0.003 * excitationLogMinus30 * excitationLogMinus30;
                }

                //convert to linear gain
                gain = decibelsToPower(gain);

                //active filter output
                for (uint j = 0; j < wActive_[i].size(); ++j)
                    excitationLinA += wActive_[i][j] * inputSpectrum[j];
                excitationLinA *= gain;

                //excitation pattern
                Real excitation = scalingFactor_ * (excitationLinP + excitationLinA);

                if (isExcitationPatternInterpolated_)
                    logExcitation_[i] = log(excitation + 1e-10);
                else
                    outputExcitationPattern[i] = excitation;
            }

            //Interpolate to estimate 0.1~Cam res excitation pattern
            if (isExcitationPatternInterpolated_)
            {
                spline_.set_points(cams_, logExcitation_, isInterpolationCubic_);

                for (int i = 0; i < 388; ++i)
                    outputExcitationPattern[i] = exp(spline_(camLo_ + i * 0.1));
            }
        }
    }

    void DoubleRoexBank::resetInternal(){};
}
