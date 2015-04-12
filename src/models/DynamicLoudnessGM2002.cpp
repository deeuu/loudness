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

#include "../thirdParty/cnpy/cnpy.h"
#include "../support/AuditoryTools.h"
#include "../modules/FrameGenerator.h"
#include "../modules/Butter.h"
#include "../modules/FIR.h"
#include "../modules/IIR.h"
#include "../modules/Window.h"
#include "../modules/PowerSpectrum.h"
#include "../modules/CompressSpectrum.h"
#include "../modules/WeightSpectrum.h"
#include "../modules/FastRoexBank.h"
#include "../modules/RoexBankANSIS342007.h"
#include "../modules/SpecificLoudnessANSIS342007.h"
#include "../modules/BinauralInhibitionMG2007.h"
#include "../modules/InstantaneousLoudness.h"
#include "../modules/ARAverager.h"
#include "DynamicLoudnessGM2002.h"

namespace loudness{

    DynamicLoudnessGM2002::DynamicLoudnessGM2002(const string& pathToFilterCoefs) :
        Model("DynamicLoudnessGM2002", true),
        pathToFilterCoefs_(pathToFilterCoefs)
    {
        configureModelParameters("recentAndFaster");
    }

    DynamicLoudnessGM2002::DynamicLoudnessGM2002() :
        Model("DynamicLoudnessGM2002", true),
        pathToFilterCoefs_("")
    {
        configureModelParameters("recentAndFaster");
    }
    
    DynamicLoudnessGM2002::~DynamicLoudnessGM2002()
    {
    }

    void DynamicLoudnessGM2002::setFirstSampleAtWindowCentre(bool isFirstSampleAtWindowCentre)
    {
        isFirstSampleAtWindowCentre_ = isFirstSampleAtWindowCentre;
    }
    
    void DynamicLoudnessGM2002::setHPFUsed(bool isHPFUsed)
    {
        isHPFUsed_ = isHPFUsed;
    }

    void DynamicLoudnessGM2002::setResponseDiffuseField(bool isResponseDiffuseField)
    {
        isResponseDiffuseField_ = isResponseDiffuseField;
    }

    void DynamicLoudnessGM2002::setPresentationDiotic(bool isPresentationDiotic)
    {
        isPresentationDiotic_ = isPresentationDiotic;
    }

    void DynamicLoudnessGM2002::setBinauralInhibitionUsed(bool isBinauralInhibitionUsed)
    {
        isBinauralInhibitionUsed_ = isBinauralInhibitionUsed;
    }

    void DynamicLoudnessGM2002::setSpectrumSampledUniformly(bool isSpectrumSampledUniformly)
    {
        isSpectrumSampledUniformly_ = isSpectrumSampledUniformly;
    }

    void DynamicLoudnessGM2002::setExcitationPatternInterpolated(bool isExcitationPatternInterpolated)
    {
        isExcitationPatternInterpolated_ = isExcitationPatternInterpolated;
    }

    void DynamicLoudnessGM2002::setFilterSpacing(Real filterSpacing)
    {
        filterSpacing_ = filterSpacing;
    }
    void DynamicLoudnessGM2002::setCompressionCriterion(Real compressionCriterion)
    {
        compressionCriterion_ = compressionCriterion;
    }

    void DynamicLoudnessGM2002::setPathToFilterCoefs(string pathToFilterCoefs)
    {
        pathToFilterCoefs_ = pathToFilterCoefs;
    }

    void DynamicLoudnessGM2002::setRoexBankFast(bool isRoexBankFast)
    {
        isRoexBankFast_ = isRoexBankFast;
    }

    void DynamicLoudnessGM2002::setSpecificLoudnessANSIS342007(bool isSpecificLoudnessANSIS342007)
    {
        isSpecificLoudnessANSIS342007_ = isSpecificLoudnessANSIS342007;
    }

    void DynamicLoudnessGM2002::configureSmoothingTimes(const string& author)
    {
        if (author == "GM2002")
        {
            attackTimeSTL_ = -0.001/log(1-0.045);
            releaseTimeSTL_ = -0.001/log(1-0.02);
            attackTimeLTL_ = -0.001/log(1-0.01);
            releaseTimeLTL_ = -0.001/log(1-0.0005);
        }
        else if (author == "MGS2003")
        {
            attackTimeSTL_ = -0.001/log(1-0.045);
            releaseTimeSTL_  = -0.001/log(1-0.02);
            attackTimeLTL_ = -0.001/log(1-0.01);
            releaseTimeLTL_ = -0.001/log(1-0.005);
        }
        else
        {
            configureSmoothingTimes("GM2002");
        }
    }
    
    void DynamicLoudnessGM2002::configureModelParameters(const string& setName)
    {
        //common to all
        setRate(1000);
        setHPFUsed(false);
        setResponseDiffuseField(false);
        setSpectrumSampledUniformly(true);
        setExcitationPatternInterpolated(false);
        setFilterSpacing(0.25);
        setCompressionCriterion(0.0);
        setRoexBankFast(false);
        setSpecificLoudnessANSIS342007(false);
        setFirstSampleAtWindowCentre(true);
        setPresentationDiotic(true);
        setBinauralInhibitionUsed(true);
        configureSmoothingTimes("GM2002");
                
        if (setName != "GM2002")
        {
            if (setName == "faster")
            {
                setRoexBankFast(true);
                setExcitationPatternInterpolated(true);
                setCompressionCriterion(0.3);
                LOUDNESS_DEBUG(name_ << ": Using faster params for Glasberg and Moore's 2002 model.");
            }
            else if (setName == "recent")
            {
                configureSmoothingTimes("MGS2003");
                setSpecificLoudnessANSIS342007(true);
                LOUDNESS_DEBUG(name_
                        << ": Using updated "
                        << "time-constants from 2003 paper and high-level specific "
                        << "loudness equation (ANSI S3.4 2007)."); 
            }
            else if (setName == "recentAndFaster")
            {
                configureSmoothingTimes("GM2003");
                setSpecificLoudnessANSIS342007(true);
                setRoexBankFast(true);
                setExcitationPatternInterpolated(true);
                setCompressionCriterion(0.3);
                LOUDNESS_DEBUG(name_
                        << ": Using faster params and "
                        << "updated time-constants from 2003 paper and "
                        << "high-level specific loudness equation (ANSI S3.4 2007).");
            }
            else
            {
                LOUDNESS_DEBUG(name_
                        << ": Using original params from Glasberg and Moore 2002.");
            }
        }
    }

    bool DynamicLoudnessGM2002::initializeInternal(const SignalBank &input)
    {
        /*
         * Outer-Middle ear filter 
         */  

        //if filter coefficients have not been provided
        //use spectral weighting to approximate outer and middle ear response
        bool weightSpectrum = false;
        if (pathToFilterCoefs_.empty())
        {
            LOUDNESS_WARNING(name_ 
                    << ": No filter coefficients, opting to weight power spectrum.");

            weightSpectrum = true; 

            //should we use for useHpf for low freqs? default is true
            if (isHPFUsed_)
            {
                modules_.push_back(unique_ptr<Module> (new Butter(3, 0, 50.0))); 
                outputNames_.push_back("HPF");
                LOUDNESS_DEBUG(name_ << ": Using HPF.");
            }
        }
        else { //otherwise, load them

            //load numpy array holding the filter coefficients
            cnpy::NpyArray arr = cnpy::npy_load(pathToFilterCoefs_);
            Real *data = reinterpret_cast<Real*> (arr.data);

            //check if filter is IIR or FIR
            bool iir = false;
            if(arr.shape[0]==2)
                iir = true;

            //load the coefficients
            RealVec bCoefs, aCoefs;
            for(unsigned int i=0; i<arr.shape[1];i++)
            {
                bCoefs.push_back(data[i]);
                if(iir)
                    aCoefs.push_back(data[i+arr.shape[1]]);
            }
            
            //create module
            if(iir)
            {
                modules_.push_back(unique_ptr<Module> 
                        (new IIR(bCoefs, aCoefs))); 
                outputNames_.push_back("IIR");
            }
            else
            {
                modules_.push_back(unique_ptr<Module>
                        (new FIR(bCoefs))); 
                outputNames_.push_back("FIR");
            }

            //clean up
            delete [] data;
        }

        /*
         * Multi-resolution spectrogram
         */
        RealVec bandFreqsHz {10, 80, 500, 1250, 2540, 4050, 15001};

        //window spec
        RealVec windowSizeSecs {0.064, 0.032, 0.016, 0.008, 0.004, 0.002};
        vector<int> windowSizeSamples(6,0);
        //round to nearest sample and force to be even such that centre samples
        //are aligned.
        for (int w = 0; w < 6; w++)
        {
            windowSizeSamples[w] = (int)round(windowSizeSecs[w] * input.getFs());
            windowSizeSamples[w] += windowSizeSamples[w]%2;
        }

        //Frame generator
        int hopSize = int(input.getFs() / rate_);
        modules_.push_back(unique_ptr<Module> 
                (new FrameGenerator(windowSizeSamples[0], hopSize, isFirstSampleAtWindowCentre_)));
        outputNames_.push_back("FrameGenerator");

        //windowing: Periodic hann window
        modules_.push_back(unique_ptr<Module>
                (new Window("hann", windowSizeSamples, true)));
        outputNames_.push_back("HannWindows");

        //power spectrum
        modules_.push_back(unique_ptr<Module> 
                (new PowerSpectrum(bandFreqsHz, windowSizeSamples, isSpectrumSampledUniformly_))); 
        outputNames_.push_back("PowerSpectrum");

        /*
         * Compression
         */
        if(compressionCriterion_ > 0)
        {
            modules_.push_back(unique_ptr<Module>
                    (new CompressSpectrum(compressionCriterion_))); 
            outputNames_.push_back("CompressedPowerSpectrum");
        }

        /*
         * Spectral weighting if necessary
         */
        if (weightSpectrum)
        {
            string middleEar = "ANSIS342007";
            string outerEar = "ANSIS342007_FREEFIELD";
            if (isHPFUsed_)
                middleEar = "ANSIS342007_HPF";
            if (isResponseDiffuseField_)
                outerEar = "ANSIS342007_DIFFUSEFIELD";

            modules_.push_back(unique_ptr<Module> 
                    (new WeightSpectrum(middleEar, outerEar))); 
            outputNames_.push_back("WeightedPowerSpectrum");
        }

        /*
         * Roex filters
         */
        if(isRoexBankFast_)
        {
            modules_.push_back(unique_ptr<Module>
                    (new FastRoexBank(filterSpacing_, isExcitationPatternInterpolated_)));
        }
        else
        {
            modules_.push_back(unique_ptr<Module> 
                    (new RoexBankANSIS342007(1.8, 38.9, filterSpacing_)));
        }
        outputNames_.push_back("ExcitationPattern");
        
        /*
         * Specific loudness
         */
        isBinauralInhibitionUsed_ = isBinauralInhibitionUsed_ * (input.getNEars() == 2);
        modules_.push_back(unique_ptr<Module>
                (new SpecificLoudnessANSIS342007(isSpecificLoudnessANSIS342007_,
                                                 isBinauralInhibitionUsed_)));
        outputNames_.push_back("SpecificLoudnessPattern");

        /*
         * Binaural inhibition
         */
        if (isBinauralInhibitionUsed_)
        {
            modules_.push_back(unique_ptr<Module>
                (new BinauralInhibitionMG2007));
            outputNames_.push_back("InhibitedSpecificLoudnessPattern");
        }
        else
        {
            LOUDNESS_DEBUG(name_ << ": No binaural inhibition.");
        }

        /*
        * Instantaneous loudness
        */   
        modules_.push_back(unique_ptr<Module>
                (new InstantaneousLoudness(1.0, isPresentationDiotic_)));
        outputNames_.push_back("InstantaneousLoudness");

        /*
         * Short-term loudness
         */
        modules_.push_back(unique_ptr<Module>
                (new ARAverager(attackTimeSTL_, releaseTimeSTL_)));
        outputNames_.push_back("ShortTermLoudness");

        /*
         * Long-term loudness
         */
        modules_.push_back(unique_ptr<Module>
                (new ARAverager(attackTimeLTL_, releaseTimeLTL_)));
        outputNames_.push_back("LongTermLoudness");
        
        //configure targets
        configureLinearTargetModuleChain();

        return 1;
    }
}
