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

#include "../modules/WeightSpectrum.h"
#include "../modules/DoubleRoexBank.h"
#include "../modules/BinauralInhibitionMG2007.h"
#include "../modules/InstantaneousLoudness.h"
#include "StationaryLoudnessCHGM2011.h"

namespace loudness{

    StationaryLoudnessCHGM2011::StationaryLoudnessCHGM2011() :
        Model("StationaryLoudnessCHGM2011", false)
    {
        //Default parameters
        setOuterEarFilter(OME::Filter::ANSIS342007_FREEFIELD);
        setFilterSpacingInCams(0.1);
        setPresentationDiotic(true);
        setBinauralInhibitionUsed(true);
        setSpecificLoudnessOutput(true);
    }

    StationaryLoudnessCHGM2011::~StationaryLoudnessCHGM2011()
    {}

    void StationaryLoudnessCHGM2011::setPresentationDiotic(bool isPresentationDiotic)
    {
        isPresentationDiotic_ = isPresentationDiotic;
    }

    void StationaryLoudnessCHGM2011::setBinauralInhibitionUsed(bool isBinauralInhibitionUsed)
    {
        isBinauralInhibitionUsed_ = isBinauralInhibitionUsed;
    }

    void StationaryLoudnessCHGM2011::setOuterEarFilter(const OME::Filter outerEarFilter)
    {
        outerEarFilter_ = outerEarFilter;
    }

    void StationaryLoudnessCHGM2011::setFilterSpacingInCams(Real filterSpacingInCams)
    {
        filterSpacingInCams_ = filterSpacingInCams;
    }

    void StationaryLoudnessCHGM2011::setSpecificLoudnessOutput(bool isSpecificLoudnessOutput)
    {
        isSpecificLoudnessOutput_ = isSpecificLoudnessOutput;
    }

    bool StationaryLoudnessCHGM2011::initializeInternal(const SignalBank &input)
    {
        /*
         * Weighting filter
         */
        modules_.push_back(unique_ptr<Module>
                (new WeightSpectrum(OME::CHGM2011_MIDDLE_EAR, outerEarFilter_))); 

        // Set up scaling factors depending on output config
        Real doubleRoexBankfactor, instantaneousLoudnessFactor;
        if (isSpecificLoudnessOutput_)
        {
            doubleRoexBankfactor = 1.53e-8;
            instantaneousLoudnessFactor = 1.0;
            LOUDNESS_DEBUG(name_ << ": Excitation pattern will be scaled for specific loudness");
        }
        else
        {
            doubleRoexBankfactor = 1.0;
            instantaneousLoudnessFactor = 1.53e-8;
        }

        isBinauralInhibitionUsed_ = isBinauralInhibitionUsed_ * (input.getNEars() == 2) * isSpecificLoudnessOutput_;
        if (isBinauralInhibitionUsed_)
            doubleRoexBankfactor /= 0.75;

        modules_.push_back(unique_ptr<Module>
                (new DoubleRoexBank(1.5, 40.2,
                                    filterSpacingInCams_,
                                    doubleRoexBankfactor,
                                    false,
                                    false)));
        /*
         * Binaural inhibition
         */
        if (isBinauralInhibitionUsed_)
        {
            modules_.push_back(unique_ptr<Module> 
                    (new BinauralInhibitionMG2007));
        }
        else
        {
            LOUDNESS_DEBUG(name_ << ": No binaural inhibition.");
        }
        outputModules_["SpecificLoudness"] = modules_.back().get();
        
        /*
        * Instantaneous loudness
        */   
        modules_.push_back(unique_ptr<Module>
                (new InstantaneousLoudness(instantaneousLoudnessFactor, 
                                           isPresentationDiotic_)));
        outputModules_["InstantaneousLoudness"] = modules_.back().get();

        //configure targets
        configureLinearTargetModuleChain();

        return 1;
    }
}
