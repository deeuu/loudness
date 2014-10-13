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

#include "SpecificLoudnessGM.h"
#include "../Support/AuditoryTools.h"

namespace loudness{

    SpecificLoudnessGM::SpecificLoudnessGM(bool ansiS3407) :
        Module("SpecificLoudnessGM"),
        ansiS3407_(ansiS3407)
    {}

    SpecificLoudnessGM::~SpecificLoudnessGM() {}

    bool SpecificLoudnessGM::initializeInternal(const SignalBank &input)
    {
        //c value from ANSI 2007
        cParam_ = 0.046871;

        //Number of filters below 500Hz
        nFiltersLT500_ = 0;

        Real eThrqdB500Hz = IntExc(500);
        //fill loudness parameter vectors
        Real fcPrev = input.getCentreFreq(0);
        for(int i=0; i<input.getNChannels(); i++)
        {
            Real fc = input.getCentreFreq(i);

            if(fc<fcPrev)
                LOUDNESS_ERROR("SpecificLoudnessGM: Centre frequencies are not in ascending order!");
            fcPrev = fc;

            if(fc<500)
            {
                Real eThrqdB = IntExc(fc);
                eThrqParam_.push_back(pow(10,eThrqdB/10.0));
                Real gdB = eThrqdB500Hz-eThrqdB;
                gParam_.push_back(pow(10, gdB/10.0));
                aParam_.push_back(GdBToA(gdB));
                alphaParam_.push_back(GdBToAlpha(gdB));
                /* 
                LOUDNESS_DEBUG("SpecificLoudnessGM: eThrq: " <<
                        eThrqParam_[nFiltersLT500_] << ", gdB: " << gdB << ", A: "
                        << aParam_[nFiltersLT500_] << ", alpha: " <<
                        alphaParam_[nFiltersLT500_]);
                */
                nFiltersLT500_++;
            }
        }
        LOUDNESS_DEBUG("SpecificLoudnessGM: number of filters <500 Hz: " << nFiltersLT500_);

        //output SignalBank
        output_.initialize(input);

        return 1;
    }

    void SpecificLoudnessGM::processInternal(const SignalBank &input)
    {
        Real excLin, sl=0.0;

        for(int i=0; i<input.getNChannels(); i++)
        {
            excLin = input.getSample(i,0);

            //checked out 2.4.14
            //high level
            if (excLin > 1e10)
            {
                if(ansiS3407_)
                    sl = pow((excLin/1.0707),0.2);
                else
                    sl = pow((excLin/1.04e6),0.5);
            }
            else if(i<nFiltersLT500_) //low freqs
            { 
                if(excLin>eThrqParam_[i]) //medium level
                {
                    sl = (pow(gParam_[i]*excLin+aParam_[i], alphaParam_[i])-
                            pow(aParam_[i],alphaParam_[i]));
                }
                else //low level
                {
                    sl = pow((2*excLin)/(excLin+eThrqParam_[i]), 1.5)*
                        (pow(gParam_[i]*excLin+aParam_[i],alphaParam_[i])
                            -pow(aParam_[i],alphaParam_[i]));
                }
            }
            else //high freqs (variables are constant >= 500 Hz)
            { 
                if(excLin>2.3604782331805771) //medium level
                {
                    sl = pow(excLin+4.72096, 0.2)-1.3639739128330546;
                } 
                else //low level
                {
                    sl = pow((2*excLin)/(excLin+2.3604782331805771), 1.5)*
                        (pow(excLin+4.72096, 0.2)-1.3639739128330546);
                }
            }
            
            output_.setSample(i, 0, cParam_*sl);
        }
    }

    void SpecificLoudnessGM::resetInternal(){};
}



