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
#include "../modules/InstantaneousLoudness.h"
#include "SteadyStateLoudnessANSIS342007.h"

namespace loudness{

    SteadyStateLoudnessANSIS342007::SteadyStateLoudnessANSIS342007() :
        Model("SteadyStateLoudnessANSIS342007", false)
    {
        //Default parameters
        setPresentationDiotic(true);
        setResponseDiffuseField(false);
        setfilterSpacingInCams(0.1);
    }

    SteadyStateLoudnessANSIS342007::~SteadyStateLoudnessANSIS342007()
    {
    }

    void SteadyStateLoudnessANSIS342007::setPresentationDiotic(bool isPresentationDiotic)
    {
        isPresentationDiotic_ = isPresentationDiotic;
    }

    void SteadyStateLoudnessANSIS342007::setResponseDiffuseField(bool isResponseDiffuseField)
    {
        isResponseDiffuseField_ = isResponseDiffuseField;
    }

    void SteadyStateLoudnessANSIS342007::setfilterSpacingInCams(Real filterSpacingInCams)
    {
        filterSpacingInCams_ = filterSpacingInCams;
    }

    bool SteadyStateLoudnessANSIS342007::initializeInternal(const SignalBank &input)
    {

        /*
         * Weighting filter
         */
        string middleEar = "ANSIS342007";
        string outerEar = "ANSIS342007_FREEFIELD";
        if(isResponseDiffuseField_)
            outerEar = "ANSIS342007_DIFFUSEFIELD";

        modules_.push_back(unique_ptr<Module>
                (new WeightSpectrum(middleEar, outerEar))); 
        outputNames_.push_back("WeightedPowerSpectrum");

        /*
         * Roex filters
         */
        modules_.push_back(unique_ptr<Module>
                (new RoexBankANSIS342007(1.8, 38.9, filterSpacingInCams_)));
        outputNames_.push_back("ExcitationPattern");
        
        /*
         * Specific loudness using high level modification
         */
        modules_.push_back(unique_ptr<Module>
                (new SpecificLoudnessANSIS342007(true, false)));
        outputNames_.push_back("SpecificLoudnessPattern");

        /*
        * Loudness integration 
        */   
        modules_.push_back(unique_ptr<Module>
                (new InstantaneousLoudness(1.0, isPresentationDiotic_)));
        outputNames_.push_back("InstantaneousLoudness");

        //configure targets
        configureLinearTargetModuleChain();

        return 1;
    }
}
