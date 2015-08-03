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

#include "../modules/OctaveBank.h"
#include "../modules/MainLoudnessDIN456311991.h"
#include "../modules/InstantaneousLoudnessDIN456311991.h"
#include "StationaryLoudnessDIN456311991.h"

namespace loudness{

    StationaryLoudnessDIN456311991::StationaryLoudnessDIN456311991(
            bool isPresentationDiffuseField,
            bool isOutputRounded) :
        Model("StationaryLoudnessDIN456311991", false),
        isPresentationDiffuseField_ (isPresentationDiffuseField),
        isOutputRounded_ (isOutputRounded)
    {}

    StationaryLoudnessDIN456311991::~StationaryLoudnessDIN456311991()
    {}

    bool StationaryLoudnessDIN456311991::initializeInternal(const SignalBank &input)
    {
        /*
         * Third octave filters
         */
        modules_.push_back(unique_ptr<Module>
                (new OctaveBank(3, 2, true, true)));
        
        /*
         * Main loudness
         */
        modules_.push_back(unique_ptr<Module>
                (new MainLoudnessDIN456311991 (isPresentationDiffuseField_)));

        /*
         * Instantaneous loudness
         */   
        modules_.push_back(unique_ptr<Module> 
                (new InstantaneousLoudnessDIN456311991));
        outputModules_["InstantaneousLoudness"] = modules_.back().get();

        //configure targets
        configureLinearTargetModuleChain();

        return 1;
    }
}
