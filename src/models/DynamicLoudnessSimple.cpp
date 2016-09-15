/*
 * Copyright (C) 2014 Dominic Ward <contactdominicward@CHail.com>
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

#include "../modules/FrameGenerator.h"
#include "../modules/Window.h"
#include "../modules/PowerSpectrum.h"
#include "../modules/CompressSpectrum.h"
#include "../modules/WeightSpectrum.h"
#include "../modules/HighpassSpectrum.h"
#include "../modules/FixedRoexBank.h"
#include "../modules/SimpleLoudness.h"
#include "../modules/ARAverager.h"
#include "DynamicLoudnessSimple.h"

namespace loudness{

    DynamicLoudnessSimple::DynamicLoudnessSimple() :
        Model("DynamicLoudnessSimple", true)
    {
        configureModelParameters("Faster");
    }

    DynamicLoudnessSimple::~DynamicLoudnessSimple()
    {}

    void DynamicLoudnessSimple::setFirstSampleAtWindowCentre(bool isFirstSampleAtWindowCentre)
    {
        isFirstSampleAtWindowCentre_ = isFirstSampleAtWindowCentre;
    }
 
    void DynamicLoudnessSimple::setPresentationDiotic(bool isPresentationDiotic)
    {
        isPresentationDiotic_ = isPresentationDiotic;
    }

    void DynamicLoudnessSimple::setFilterSpacingInCams(Real filterSpacingInCams)
    {
        filterSpacingInCams_ = filterSpacingInCams;
    }

    void DynamicLoudnessSimple::setCompressionCriterionInCams(Real compressionCriterionInCams)
    {
        compressionCriterionInCams_ = compressionCriterionInCams;
    }

    void DynamicLoudnessSimple::setOuterEarFilter(const OME::Filter& outerEarFilter)
    {
        outerEarFilter_ = outerEarFilter;
    }

    void DynamicLoudnessSimple::setAlpha(Real alpha)
    {
        alpha_ = alpha;
    }

    void DynamicLoudnessSimple::setFactor(Real factor)
    {
        factor_ = factor;
    }

    void DynamicLoudnessSimple::setRoexLevel(Real roexLevel)
    {
        roexLevel_ = roexLevel;
    }

    void DynamicLoudnessSimple::setAttackTime(Real attackTime)
    {
        attackTime_ = attackTime;
    }

    void DynamicLoudnessSimple::setReleaseTime(Real releaseTime)
    {
        releaseTime_ = releaseTime;
    }

    void DynamicLoudnessSimple::setFilterFC (Real fc)
    {
        fc_ = fc;
    }

    void DynamicLoudnessSimple::setFilterGain (Real gain)
    {
        gain_ = gain;
    }

    void DynamicLoudnessSimple::setFilterSlope (Real slope)
    {
        slope_ = slope;
    }

    void DynamicLoudnessSimple::configureModelParameters(const string& setName)
    {
        //common to all
        setRate (500);
        setFirstSampleAtWindowCentre (true);
        setFilterSpacingInCams (1.0);
        setCompressionCriterionInCams (0.3);
        setPresentationDiotic (true);
        setAlpha (2.0/3.0);
        setFactor (1.0);
        setRoexLevel (70.0);
        setAttackTime (0.050);
        setReleaseTime (0.800);
        setFilterFC (100.0);
        setFilterGain (3.0);
        setFilterSlope (3.5);
        setOuterEarFilter(OME::LML_FREEFIELD);
    }

    bool DynamicLoudnessSimple::initializeInternal(const SignalBank &input)
    {
        /*
         * Multi-resolution spectrogram
         */
        RealVec bandFreqsHz {10, 935, 2095, 16001};

        //window spec
        RealVec windowSizeSecs {0.064, 0.032, 0.016};
        vector<int> windowSizeSamples(3,0);
        //round to nearest sample and force to be even such that centre samples
        //are aligned (using periodic Hann window)
        for(int w=0; w<3; w++)
        {
            windowSizeSamples[w] = (int)round(windowSizeSecs[w]
                                              * input.getFs());
            windowSizeSamples[w] += windowSizeSamples[w] % 2;
        }
        
        // hop size to the nearest sample
        int hopSize = round(input.getFs() / rate_);
        
        modules_.push_back(unique_ptr<Module> 
                (new FrameGenerator(windowSizeSamples[0],
                                    hopSize,
                                    isFirstSampleAtWindowCentre_)));

        //windowing: Periodic hann window
        modules_.push_back(unique_ptr<Module>
                (new Window(Window::HANN, windowSizeSamples, true)));

        modules_.push_back(unique_ptr<Module> 
                (new PowerSpectrum(bandFreqsHz,
                                   windowSizeSamples,
                                   true,
                                   PowerSpectrum::AVERAGE_POWER,
                                   2e-5)));
        /*
         * Compression
         */
        if((compressionCriterionInCams_ > 0))
        {
            modules_.push_back(unique_ptr<Module>
                    (new CompressSpectrum(compressionCriterionInCams_))); 
        }

        /*
         * Spectral weighting
         */
        if (outerEarFilter_ != OME::NONE)
        {
            modules_.push_back(unique_ptr<Module> 
                    (new WeightSpectrum(OME::NONE, outerEarFilter_)));
        }

        modules_.push_back(unique_ptr<Module>
                (new HighpassSpectrum(fc_, gain_, slope_))); 


        /*
         * Roex filters
         */

        // Set up scaling factors depending on output config
        modules_.push_back(unique_ptr<Module>
                (new FixedRoexBank(1.5, 39.5,
                                    filterSpacingInCams_,
                                    roexLevel_)));
        outputModules_["Excitation"] = modules_.back().get();

        /*
        * Instantaneous loudness
        */   
        modules_.push_back(unique_ptr<Module>
                (new SimpleLoudness(alpha_,
                                    factor_,
                                    isPresentationDiotic_)));
        outputModules_["SpecificLoudness"] = modules_.back().get();

        /*
         * Short-term loudness
         */
        modules_.push_back(unique_ptr<Module>
                (new ARAverager(attackTime_, releaseTime_)));
        outputModules_["ShortTermLoudness"] = modules_.back().get();

        //configure targets
        configureLinearTargetModuleChain();

        return 1;
    }
}
