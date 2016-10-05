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

#include "../modules/Biquad.h"
#include "../modules/FrameGenerator.h"
#include "../modules/Window.h"
#include "../modules/PowerSpectrum.h"
#include "../modules/CompressSpectrum.h"
#include "../modules/WeightSpectrum.h"
#include "../modules/HighpassSpectrum.h"
#include "../modules/FixedRoexBank.h"
#include "../modules/SimpleLoudness.h"
#include "../modules/ARAverager.h"
#include "../modules/PeakFollower.h"
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

    void DynamicLoudnessSimple::setMiddleEarFilter(const OME::Filter& middleEarFilter)
    {
        middleEarFilter_ = middleEarFilter;
    }

    void DynamicLoudnessSimple::setOffsetKDB (Real offsetKDB)
    {
        offsetKDB_ = offsetKDB;
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

    void DynamicLoudnessSimple::setMaskingTol (Real maskingTol)
    {
        maskingTol_ = maskingTol;
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

    void DynamicLoudnessSimple::setUseMultiSourceMasking (bool useMultiSourceMasking)
    {
        useMultiSourceMasking_ = useMultiSourceMasking;
    }

    void DynamicLoudnessSimple::setMaskK(Real maskK)
    {
        maskK_ = maskK;
    }

    void DynamicLoudnessSimple::setMaskExponent (Real maskExponent)
    {
        maskExponent_ = maskExponent;
    }

    void DynamicLoudnessSimple::setUseRLB (bool useRLB)
    {
        useRLB_ = useRLB;
    }

    void DynamicLoudnessSimple::setUsePreFilter (bool usePreFilter)
    {
        usePreFilter_ = usePreFilter;
    }

    void DynamicLoudnessSimple::setHighpassSpectrum (bool highpassSpectrum)
    {
        highpassSpectrum_ = highpassSpectrum;
    }

    void DynamicLoudnessSimple::setUseTemporalMasking(bool useTemporalMasking)
    {
        useTemporalMasking_ = useTemporalMasking;
    }

    void DynamicLoudnessSimple::setTemporalMaskingTau(Real temporalMaskingTau)
    {
        temporalMaskingTau_ = temporalMaskingTau;
    }

    void DynamicLoudnessSimple::configureModelParameters(const string& setName)
    {
        //common to all
        setRate (500);
        setFirstSampleAtWindowCentre (true);
        setFilterSpacingInCams (1.0);
        setCompressionCriterionInCams (0.3);
        setPresentationDiotic (true);
        setRoexLevel (70.0);
        setUseRLB (true);
        setUsePreFilter (true);
        setAttackTime (0.1);
        setReleaseTime (1.6);
        setAlpha (0.5);
        setFactor (6.48176074655e-06);
        setHighpassSpectrum (false);
        setOuterEarFilter(OME::NONE);
        setMiddleEarFilter(OME::NONE);

        setUseTemporalMasking (false);
        setTemporalMaskingTau (0.05);

        setUseMultiSourceMasking (false);
        setMaskingTol (3.0);
        setOffsetKDB (0.0);
        setMaskK (0.0);
        setMaskExponent (2.0);
        setFilterFC (450.0);
        setFilterGain (1.0);
        setFilterSlope (3.5);
    }

    bool DynamicLoudnessSimple::initializeInternal(const SignalBank &input)
    {
        if (useRLB_)
            modules_.push_back(unique_ptr<Module> (new Biquad("RLB")));

        if (usePreFilter_)
            modules_.push_back(unique_ptr<Module> (new Biquad("prefilter")));

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
                    (new WeightSpectrum(middleEarFilter_, outerEarFilter_)));
        }

        if (highpassSpectrum_)
        {
            modules_.push_back(unique_ptr<Module>
                    (new HighpassSpectrum(fc_, 1.0, slope_)));
        }

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
        /*
         * Short-term loudness
         */
        modules_.push_back(unique_ptr<Module>
                (new ARAverager(attackTime_, releaseTime_)));
        outputModules_["ShortTermLoudness"] = modules_.back().get();

        // configure targets
        configureLinearTargetModuleChain();

        /*
        if (input.getNSources() > 1)
        {
            if (useMultiSourceMasking_)
            {
                modules_.push_back(unique_ptr<Module>
                    (new MultiSourceMasking()));
            }
            else
            {
                modules_.push_back(unique_ptr<Module>
                    (new MultiSourceMasking3(offsetKDB_)));
            }

            outputModules_["Excitation"] -> addTargetModule (
                    *modules_.back().get());
            int moduleIdx = modules_.size() - 1;

            outputModules_["MultiSourceExcitation"] = modules_.back().get();

            modules_.push_back(unique_ptr<Module>
                (new SimpleLoudness(alpha_,
                                    factor_,
                                    isPresentationDiotic_)));

            modules_.push_back(unique_ptr<Module>
                (new ARAverager(attackTime_, releaseTime_)));
            outputModules_["ShortTermPartialLoudness"] = modules_.back().get();

            configureLinearTargetModuleChain(moduleIdx);
        }
        */

        return 1;
    }
}
