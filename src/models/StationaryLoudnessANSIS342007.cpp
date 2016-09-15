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
#include "../modules/MultiSourceRoexBank.h"
#include "../modules/SpecificPartialLoudnessMGB1997.h"
#include "../modules/SpecificLoudnessANSIS342007.h"
#include "../modules/SpecificLoudnessModANSIS342007.h"
#include "../modules/BinauralInhibitionMG2007.h"
#include "../modules/InstantaneousLoudness.h"
#include "StationaryLoudnessANSIS342007.h"

namespace loudness{

    StationaryLoudnessANSIS342007::StationaryLoudnessANSIS342007() :
        Model("StationaryLoudnessANSIS342007", false)
    {
        //Default parameters
        setOuterEarFilter (OME::Filter::ANSIS342007_FREEFIELD);
        setFilterSpacingInCams (0.1);
        setPresentationDiotic (true);
        setBinauralInhibitionUsed (true);
        setSpecificLoudnessANSIS342007 (true);
        setPartialLoudnessUsed (true);
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

    void StationaryLoudnessANSIS342007::setPartialLoudnessUsed(bool isPartialLoudnessUsed)
    {
        isPartialLoudnessUsed_ = isPartialLoudnessUsed;
    }

    void StationaryLoudnessANSIS342007::setOuterEarFilter(const OME::Filter outerEarFilter)
    {
        outerEarFilter_ = outerEarFilter;
    }

    void StationaryLoudnessANSIS342007::setFilterSpacingInCams(Real filterSpacingInCams)
    {
        filterSpacingInCams_ = filterSpacingInCams;
    }

    void StationaryLoudnessANSIS342007::setSpecificLoudnessANSIS342007(bool isSpecificLoudnessANSIS342007)
    {
        isSpecificLoudnessANSIS342007_ = isSpecificLoudnessANSIS342007;
    }

    void StationaryLoudnessANSIS342007::setAlpha (const RealVec& alpha)
    {
        alpha_ = alpha;
    }

    bool StationaryLoudnessANSIS342007::initializeInternal(const SignalBank &input)
    {
        /*
         * Weighting filter
         */
        modules_.push_back(unique_ptr<Module>
                (new WeightSpectrum(OME::Filter::ANSIS342007_MIDDLE_EAR, outerEarFilter_))); 

        /*
         * Roex filters -> Specific loudness
         */
        isBinauralInhibitionUsed_ = isBinauralInhibitionUsed_
                                    * (input.getNEars() == 2);
        modules_.push_back(unique_ptr<Module>
                (new RoexBankANSIS342007(1.8, 38.9, filterSpacingInCams_)));
        outputModules_["Excitation"] = modules_.back().get();

        if (!alpha_.empty())
        {
            modules_.push_back(unique_ptr<Module>
                    (new SpecificLoudnessModANSIS342007(
                        alpha_,
                        isBinauralInhibitionUsed_)));
        }
        else
        {
            modules_.push_back(unique_ptr<Module>
                    (new SpecificLoudnessANSIS342007(
                        isSpecificLoudnessANSIS342007_,
                        isBinauralInhibitionUsed_)));
        }

        /*
         * Binaural inhibition
         */
        if (isBinauralInhibitionUsed_)
            modules_.push_back(unique_ptr<Module> (new BinauralInhibitionMG2007));
        outputModules_["SpecificLoudness"] = modules_.back().get();

        /*
         * Instantaneous loudness
         */   
        modules_.push_back(unique_ptr<Module> 
                (new InstantaneousLoudness(1.0, isPresentationDiotic_)));
        outputModules_["InstantaneousLoudness"] = modules_.back().get();

        //configure targets
        configureLinearTargetModuleChain();

        // Masking conditions
        if ((input.getNSources() > 1) && (isPartialLoudnessUsed_))
        {
            LOUDNESS_DEBUG(name_ 
                           << ": Setting up modules for partial loudness...");
            // Excitation transformation based on all sources
            modules_.push_back(unique_ptr<Module>
                    (new MultiSourceRoexBank(filterSpacingInCams_)));

            /*
            modules_.push_back(unique_ptr<Module>
                (new RoexBankANSIS342007(1.8, 38.9, filterSpacingInCams_)));
            */
            int moduleIdx = modules_.size() - 1;

            // Push spectrum to second excitation transformation stage
            Module* ptrToWeightedSpectrum = modules_[0].get();
            ptrToWeightedSpectrum -> addTargetModule (*modules_.back().get());

            // Partial loudness
            modules_.push_back(unique_ptr<Module>
                    (new SpecificPartialLoudnessMGB1997(
                        true,
                        isBinauralInhibitionUsed_)));

            if (isBinauralInhibitionUsed_)
                modules_.push_back(unique_ptr<Module> (new BinauralInhibitionMG2007));
            outputModules_["SpecificPartialLoudness"] = modules_.back().get();

            modules_.push_back(unique_ptr<Module> 
                    (new InstantaneousLoudness(1.0, isPresentationDiotic_)));
            outputModules_["InstantaneousPartialLoudness"] = modules_.back().get();

            // configure targets for second (parallel) chain
            configureLinearTargetModuleChain(moduleIdx);
        }

        return 1;
    }
}
