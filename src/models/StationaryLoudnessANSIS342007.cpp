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
#include "../modules/RoexBankANSIS342007.h"
#include "../modules/SpecificLoudnessANSIS342007.h"
#include "../modules/BinauralInhibitionMG2007.h"
#include "../modules/InstantaneousLoudness.h"
#include "StationaryLoudnessANSIS342007.h"

namespace loudness{

    StationaryLoudnessANSIS342007::StationaryLoudnessANSIS342007() :
        Model("StationaryLoudnessANSIS342007", false)
    {
        //Default parameters
        setOuterEarType(OME::Filter::ANSIS342007_FREEFIELD);
        setfilterSpacingInCams(0.1);
        setPresentationDiotic(true);
        setBinauralInhibitionUsed(false);
    }

    StationaryLoudnessANSIS342007::~StationaryLoudnessANSIS342007()
    {}

    void StationaryLoudnessANSIS342007::setPresentationDiotic(bool isPresentationDiotic)
    {
        isPresentationDiotic_ = isPresentationDiotic;
    }

    void StationaryLoudnessANSIS342007::setBinauralInhibitionUsed(bool isBinauralInhibitionUsed)
    {
        isBinauralInhibitionUsed_ = isBinauralInhibitionUsed;
    }

    void StationaryLoudnessANSIS342007::setOuterEarType(const OME::Filter outerEarType)
    {
        outerEarType_ = outerEarType;
    }

    void StationaryLoudnessANSIS342007::setfilterSpacingInCams(Real filterSpacingInCams)
    {
        filterSpacingInCams_ = filterSpacingInCams;
    }

    bool StationaryLoudnessANSIS342007::initializeInternal(const SignalBank &input)
    {
        /*
         * Weighting filter
         */
        modules_.push_back(unique_ptr<Module>
                (new WeightSpectrum(OME::Filter::ANSIS342007_MIDDLE_EAR, outerEarType_))); 

        /*
         * Roex filters
         */
        modules_.push_back(unique_ptr<Module>
                (new RoexBankANSIS342007(1.8, 38.9, filterSpacingInCams_)));
        outputModules_["ExcitationPattern"] = modules_.back().get();
        
        /*
         * Specific loudness
         */
        isBinauralInhibitionUsed_ = isBinauralInhibitionUsed_ * (input.getNEars() == 2);
        modules_.push_back(unique_ptr<Module>
                (new SpecificLoudnessANSIS342007(true, isBinauralInhibitionUsed_)));

        /*
         * Binaural inhibition
         */
        if (isBinauralInhibitionUsed_)
        {
            modules_.push_back(unique_ptr<Module> (new BinauralInhibitionMG2007));
        }
        outputModules_["SpecificLoudnessPattern"] = modules_.back().get();

        /*
         * Instantaneous loudness
         */   
        modules_.push_back(unique_ptr<Module> 
                (new InstantaneousLoudness(1.0, isPresentationDiotic_)));
        outputModules_["InstantaneousLoudness"] = modules_.back().get();

        //configure targets
        configureLinearTargetModuleChain();

        return 1;
    }
}
