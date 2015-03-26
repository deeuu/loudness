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

#include "../Modules/WeightSpectrum.h"
#include "../Modules/RoexBankANSIS3407.h"
#include "../Modules/SpecificLoudnessGM.h"
#include "../Modules/IntegratedLoudnessGM.h"
#include "SteadyLoudnessANSIS3407.h"

namespace loudness{

    SteadyLoudnessANSIS3407::SteadyLoudnessANSIS3407() :
        Model("SteadyLoudnessANSIS3407", false)
    {
        //Default parameters
        setDiotic(true);
        setDiffuseField(false);
        setFilterSpacing(0.1);
    }

    SteadyLoudnessANSIS3407::~SteadyLoudnessANSIS3407()
    {
    }

    void SteadyLoudnessANSIS3407::setDiotic(bool diotic)
    {
        diotic_ = diotic;
    }

    void SteadyLoudnessANSIS3407::setDiffuseField(bool diffuseField)
    {
        diffuseField_ = diffuseField;
    }

    void SteadyLoudnessANSIS3407::setFilterSpacing(Real filterSpacing)
    {
        filterSpacing_ = filterSpacing;
    }

    bool SteadyLoudnessANSIS3407::initializeInternal(const SignalBank &input)
    {

        /*
         * Weighting filter
         */
        string middleEar = "ANSI";
        string outerEar = "ANSI_FREEFIELD";
        if(diffuseField_)
            outerEar = "ANSI_DIFFUSEFIELD";

        modules_.push_back(unique_ptr<Module>
                (new WeightSpectrum(middleEar, outerEar))); 

        /*
         * Roex filters
         */
        modules_.push_back(unique_ptr<Module>
                (new RoexBankANSIS3407(1.8, 38.9, filterSpacing_)));
        
        /*
         * Specific loudness using high level modification
         */
        modules_.push_back(unique_ptr<Module>
                (new SpecificLoudnessGM(true)));

        /*
        * Loudness integration 
        */   
        modules_.push_back(unique_ptr<Module>
                (new IntegratedLoudnessGM("SteadyState"));

        return 1;
    }
}
