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

#include "IntegratedLoudnessGM.h"
#include "../Support/AuditoryTools.h"

namespace loudness{

    IntegratedLoudnessGM::IntegratedLoudnessGM(SMOOTHTIMES author, bool diotic, Real cParam) :
        diotic_(diotic),
        cParam_(cParam),
        uniform_(false)
    {
        LOUDNESS_DEBUG("IntegratedLoudness: Constructed.");
        name_ = "IntegratedLoudness";

        loadSmoothingTimes(author);
    }

    void IntegratedLoudnessGM::loadSmoothingTimes(SMOOTHTIMES author)
    {
        switch(author)
        {
            case(GM02):
                setAttackTimeSTL(-0.001/log(1-0.045));    
                setReleaseTimeSTL(-0.001/log(1-0.02));
                setAttackTimeLTL(-0.001/log(1-0.01));
                setReleaseTimeLTL(-0.001/log(1-0.0005));
                break;
            case(GM03):
                setAttackTimeSTL(-0.001/log(1-0.045));    
                setReleaseTimeSTL(-0.001/log(1-0.02));
                setAttackTimeLTL(-0.001/log(1-0.01));
                setReleaseTimeLTL(-0.001/log(1-0.005));
                break;
            case(CH12):
                setAttackTimeSTL(0.016);    
                setReleaseTimeSTL(0.032);
                setAttackTimeLTL(0.1);
                setReleaseTimeLTL(2.0);
        }
    }

    IntegratedLoudnessGM::~IntegratedLoudnessGM()
    {
    };

    void IntegratedLoudnessGM::setAttackTimeSTL(Real attackTimeSTL)
    {
        attackTimeSTL_ = attackTimeSTL;
    }

    void IntegratedLoudnessGM::setReleaseTimeSTL(Real releaseTimeSTL)
    {
        releaseTimeSTL_ = releaseTimeSTL;
    }

    void IntegratedLoudnessGM::setAttackTimeLTL(Real attackTimeLTL)
    {
        attackTimeLTL_ = attackTimeLTL;
    }

    void IntegratedLoudnessGM::setReleaseTimeLTL(Real releaseTimeLTL)
    {
        releaseTimeLTL_ = releaseTimeLTL;
    }

    bool IntegratedLoudnessGM::initializeInternal(const SignalBank &input)
    {
        if(input.getNChannels()<1)
        {
            LOUDNESS_ERROR(name_ << ": Insufficient channels.");
            return 0;
        }

        //assumes uniformly spaced ERB filters
        camStep_ = FreqToCam(input.getCentreFreq(1))-FreqToCam(input.getCentreFreq(0));
        LOUDNESS_DEBUG(name_ << ": Filter spacing (Cams): " << camStep_);

        //if excitation pattern is sampled non-uniformly, approximate integral
        if(!uniform_)
        {
            for(int i=1; i<input.getNChannels(); i++)
            {
                //compute difference between subsequent filters on cam scale
                Real step = FreqToCam(input.getCentreFreq(i)) -
                    FreqToCam(input.getCentreFreq(i-1));
                //store
                camDif_.push_back(step);
            }

            //overwrite camStep_
            camStep_ = 1;
        }

        //if diotic presentation, multiply total loudness by 2
        if(diotic_)
        {
            cParam_ *= 2;
            LOUDNESS_DEBUG(name_ << ": Diotic presentation, loudness will be multiplied by 2.");
        }

        //update scaling factor
        cParam_ *= camStep_;

        //coefficient configuration
        timeStep_ = 1.0/input.getFrameRate();
        LOUDNESS_DEBUG(name_ << ": IntegratedLoudnessGM: Time step: " << timeStep_);
        
        //short-term loudness time-constants (alpha from paper)
        attackSTLCoef_ = 1-exp(-timeStep_/attackTimeSTL_);
        releaseSTLCoef_ = 1-exp(-timeStep_/releaseTimeSTL_);
        attackLTLCoef_ = 1-exp(-timeStep_/attackTimeLTL_);
        releaseLTLCoef_ = 1-exp(-timeStep_/releaseTimeLTL_);

        //output SignalBank
        output_.initialize(3, 1, input.getFs());
        output_.setFrameRate(input.getFrameRate());

        return 1;
    }

    void IntegratedLoudnessGM::processInternal(const SignalBank &input)
    {       
        //instantaneous loudness
        Real il = 0;
        if(uniform_)
        {
            for(int chn=0; chn<input.getNChannels(); chn++)
                il += input.getSample(chn, 0);
        }
        else
        {
            for(int chn=0; chn<input.getNChannels()-1; chn++)
            {
                il += input.getSample(chn,0)*camDif_[chn] + 0.5*camDif_[chn]*
                    (input.getSample(chn+1,0)-input.getSample(chn,0));
            }
        }

        //apply scaling factor
        il *= cParam_;

        //short-term loudness
        Real prevSTL = output_.getSample(1,0);
        Real stl = 0.0;

        if(il>prevSTL)
            stl = attackSTLCoef_*(il-prevSTL) + prevSTL;
        else
            stl = releaseSTLCoef_*(il-prevSTL) + prevSTL;

        //long-term loudness
        Real prevLTL = output_.getSample(2,0);
        Real ltl = 0.0;
        if(stl>prevLTL)
            ltl = attackLTLCoef_*(stl-prevLTL) + prevLTL;
        else
            ltl = releaseLTLCoef_*(stl-prevLTL) + prevLTL;
        
        //fill output SignalBank
        output_.setSample(0,0,il);
        output_.setSample(1,0,stl);
        output_.setSample(2,0,ltl);
    }

    //output SignalBanks are cleared so not to worry about filter state
    void IntegratedLoudnessGM::resetInternal()
    {
    }
}

