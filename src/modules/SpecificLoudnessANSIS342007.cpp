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

#include "SpecificLoudnessANSIS342007.h"

namespace loudness{

    SpecificLoudnessANSIS342007::SpecificLoudnessANSIS342007(
            bool useANSISpecificLoudness,
            bool updateParameterCForBinauralInhibition) :
        Module("SpecificLoudnessANSIS342007"),
        useANSISpecificLoudness_(useANSISpecificLoudness),
        updateParameterCForBinauralInhibition_(updateParameterCForBinauralInhibition)
    {}

    SpecificLoudnessANSIS342007::~SpecificLoudnessANSIS342007() {}

    Real SpecificLoudnessANSIS342007::internalExcitation(Real freq)
    {
        if (freq>=500)
        {
            return 3.73;
        }
        else
        {
            Real logFreq;
            if(freq<=50)
                logFreq = log10(50);
            else
                logFreq = log10(freq);

            return pow(10, (0.43068810954936998 * pow(logFreq,3) 
                        - 2.7976098820730675 * pow(logFreq,2) 
                        + 5.0738460335696969 * logFreq -
                        1.2060617476790148));
        }
    };

    Real SpecificLoudnessANSIS342007::gdBToA(Real gdB)
    {
        if(gdB>=0)
        {
            return 4.72096;
        }
        else
        {
            return -0.0000010706497192096045 * pow(gdB,5) 
                -0.000060648487122230512 * pow(gdB,4) 
                -0.0012047326575717733 * pow(gdB,3) 
                -0.0068190417911848525 * pow(gdB,2)
                -0.11847825641628305 * gdB
                + 4.7138722463497347;
        }
    }

    Real SpecificLoudnessANSIS342007::gdBToAlpha(Real gdB)
    {
        if(gdB>=0)
        {
            return 0.2;
        }
        else
        {
            return 0.000026864285714285498 * pow(gdB,2)
                -0.0020023357142857231 * gdB + 0.19993107142857139;
        }
    }

    void SpecificLoudnessANSIS342007::setParameterC(Real parameterC)
    {
        parameterC_ = parameterC;
    }

    bool SpecificLoudnessANSIS342007::initializeInternal(const SignalBank &input)
    {
        LOUDNESS_ASSERT(input.getNChannels() > 1,
                name_ << ": Insufficient number of input channels.");

        //c value from ANSI 2007
        parameterC_ = 0.046871;
        
        if (updateParameterCForBinauralInhibition_)
        {
            parameterC_ /= 0.75;
            LOUDNESS_DEBUG(name_ 
                    << ": Scaling parameter C for binaural inhibition model: "
                    << parameterC_);
        }

        //Number of filters below 500Hz
        nFiltersLT500_ = 0;

        Real eThrqdB500Hz = internalExcitation(500);
        //fill loudness parameter vectors
        for (int i = 0; i < input.getNChannels(); i++)
        {
            Real fc = input.getCentreFreq(i);
            if (fc < 500)
            {
                Real eThrqdB = internalExcitation(fc);
                eThrqParam_.push_back(pow(10, eThrqdB/10.0));
                Real gdB = eThrqdB500Hz - eThrqdB;
                parameterG_.push_back(pow(10, gdB/10.0));
                parameterA_.push_back(gdBToA(gdB));
                parameterAlpha_.push_back(gdBToAlpha(gdB));
                nFiltersLT500_++;
            }
        }

        LOUDNESS_DEBUG(name_ << ": number of filters <500 Hz: " << nFiltersLT500_);

        //output SignalBank
        output_.initialize(input);

        return 1;
    }

    void SpecificLoudnessANSIS342007::processInternal(const SignalBank &input)
    {
        for (int ear = 0; ear < input.getNEars(); ear++)
        {
            Real excLin, sl = 0.0;
            const Real* inputExcitationPattern = input.getSingleSampleReadPointer(ear, 0);
            Real* outputSpecificLoudness = output_.getSingleSampleWritePointer(ear, 0);

            for (int i = 0; i < input.getNChannels(); i++)
            {
                excLin = inputExcitationPattern[i];

                //checked out 2.4.14
                //high level
                if (excLin > 1e10)
                {
                    if (useANSISpecificLoudness_)
                        sl = pow((excLin/1.0707), 0.2);
                    else
                        sl = pow((excLin/1.04e6), 0.5);
                }
                else if (i < nFiltersLT500_) //low freqs
                { 
                    if (excLin > eThrqParam_[i]) //medium level
                    {
                        sl = (pow(parameterG_[i]*excLin+parameterA_[i], parameterAlpha_[i]) -
                                pow(parameterA_[i], parameterAlpha_[i]));
                    }
                    else //low level
                    {
                        sl = pow((2*excLin)/(excLin+eThrqParam_[i]), 1.5) *
                            (pow(parameterG_[i]*excLin+parameterA_[i], parameterAlpha_[i])
                                - pow(parameterA_[i], parameterAlpha_[i]));
                    }
                }
                else //high freqs (variables are constant >= 500 Hz)
                { 
                    if (excLin > 2.3604782331805771) //medium level
                    {
                        sl = pow(excLin+4.72096, 0.2)-1.3639739128330546;
                    } 
                    else //low level
                    {
                        sl = pow((2*excLin)/(excLin+2.3604782331805771), 1.5) *
                            (pow(excLin+4.72096, 0.2)-1.3639739128330546);
                    }
                }
                
                outputSpecificLoudness[i] = parameterC_ * sl;
            }
        }
    }

    void SpecificLoudnessANSIS342007::resetInternal(){};
}
