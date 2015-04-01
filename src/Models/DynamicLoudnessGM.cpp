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

#include "../cnpy/cnpy.h"
#include "../Support/AuditoryTools.h"
#include "../Modules/FrameGenerator.h"
#include "../Modules/Butter.h"
#include "../Modules/FIR.h"
#include "../Modules/IIR.h"
#include "../Modules/Window.h"
#include "../Modules/PowerSpectrum.h"
#include "../Modules/CompressSpectrum.h"
#include "../Modules/WeightSpectrum.h"
#include "../Modules/FastRoexBank.h"
#include "../Modules/RoexBankANSIS342007.h"
#include "../Modules/SpecificLoudnessGM.h"
#include "../Modules/IntegratedLoudnessGM.h"
#include "DynamicLoudnessGM.h"

namespace loudness{

    DynamicLoudnessGM::DynamicLoudnessGM(const string& pathToFilterCoefs) :
        Model("DynamicLoudnessGM", true),
        pathToFilterCoefs_(pathToFilterCoefs)
    {
        loadParameterSet("RecentAndFaster");
    }

    DynamicLoudnessGM::DynamicLoudnessGM() :
        Model("DynamicLoudnessGM", true),
        pathToFilterCoefs_("")
    {
        loadParameterSet("RecentAndFaster");
    }
    
    DynamicLoudnessGM::~DynamicLoudnessGM()
    {
    }

    void DynamicLoudnessGM::setStartAtWindowCentre(bool startAtWindowCentre)
    {
        startAtWindowCentre_ = startAtWindowCentre;
    }
    
    void DynamicLoudnessGM::setHpf(bool hpf)
    {
        hpf_ = hpf;
    }
    void DynamicLoudnessGM::setDiffuseField(bool diffuseField)
    {
        diffuseField_ = diffuseField;
    }

    void DynamicLoudnessGM::setUniform(bool uniform)
    {
        uniform_ = uniform;
    }
    void DynamicLoudnessGM::setInterpRoexBank(bool interpRoexBank)
    {
        interpRoexBank_ = interpRoexBank;
    }
    void DynamicLoudnessGM::setFilterSpacing(Real filterSpacing)
    {
        filterSpacing_ = filterSpacing;
    }
    void DynamicLoudnessGM::setCompressionCriterion(Real compressionCriterion)
    {
        compressionCriterion_ = compressionCriterion;
    }
    void DynamicLoudnessGM::setAnsiBank(bool ansiBank)
    {
        ansiBank_ = ansiBank;
    }
    void DynamicLoudnessGM::setPathToFilterCoefs(string pathToFilterCoefs)
    {
        pathToFilterCoefs_ = pathToFilterCoefs;
    }

    void DynamicLoudnessGM::setFastBank(bool fastBank)
    {
        fastBank_ = fastBank;
    }
    void DynamicLoudnessGM::setAnsiSpecificLoudness(bool ansiSpecificLoudness)
    {
        ansiSpecificLoudness_ = ansiSpecificLoudness;
    }

    void DynamicLoudnessGM::setSmoothingType(const string& smoothingType)
    {

        if ((smoothingType != "GM2002") && (smoothingType != "GM2003"))
            smoothingType_ = "GM2002";
        else
            smoothingType_ = smoothingType;
    }
    
    void DynamicLoudnessGM::loadParameterSet(const string& setName)
    {
        //common to all
        setRate(1000);
        setHpf(true);
        setDiffuseField(false);
        setUniform(true);
        setInterpRoexBank(false);
        setFilterSpacing(0.25);
        setCompressionCriterion(0.0);
        setFastBank(false);
        setAnsiSpecificLoudness(false);
        setSmoothingType("GM2002");
        setStartAtWindowCentre(true);

        if (setName != "GM2002")
        {
            if (setName == "Faster")
            {
                setFastBank(true);
                setInterpRoexBank(true);
                setCompressionCriterion(0.3);
                LOUDNESS_DEBUG(name_ << ": Using faster params for Glasberg and Moore's 2002 model.");
            }
            else if (setName == "Recent")
            {
                setSmoothingType("GM2003");
                setAnsiSpecificLoudness(true);
                LOUDNESS_DEBUG(name_ << ": Using updated " <<
                        "time-constants from 2003 paper and high-level specific " << 
                        "loudness equation (2007)."); 
            }
            else if (setName == "RecentAndFaster")
            {
                setSmoothingType("GM2003");
                setAnsiSpecificLoudness(true);
                setFastBank(true);
                setInterpRoexBank(true);
                setCompressionCriterion(0.3);
                LOUDNESS_DEBUG(name_ << ": Using faster params and " << 
                        "updated time-constants from 2003 paper and " << 
                        "high-level specific loudness equation (2007).");
            }
            else
            {
                LOUDNESS_DEBUG(name_ << ": Using original params from Glasberg and Moore 2002.");
            }
        }
    }

    bool DynamicLoudnessGM::initializeInternal(const SignalBank &input)
    {
        /*
         * Outer-Middle ear filter 
         */  

        //if filter coefficients have not been provided
        //use spectral weighting to approximate outer and middle ear response
        bool weightSpectrum = false;
        if(pathToFilterCoefs_.empty())
        {
            LOUDNESS_WARNING(name_ 
                    << ": No filter coefficients, opting to weight power spectrum.");

            weightSpectrum = true; 

            //should we use for HPF for low freqs? default is true
            if(hpf_)
            {
                modules_.push_back(unique_ptr<Module> (new Butter(3, 0, 50.0))); 
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
                modules_.push_back(unique_ptr<Module> 
                        (new IIR(bCoefs, aCoefs))); 
            else
                modules_.push_back(unique_ptr<Module>
                        (new FIR(bCoefs))); 

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
                (new FrameGenerator(windowSizeSamples[0], hopSize, startAtWindowCentre_)));

        //windowing: Periodic hann window
        modules_.push_back(unique_ptr<Module>
                (new Window("hann", windowSizeSamples, true)));

        //power spectrum
        modules_.push_back(unique_ptr<Module> 
                (new PowerSpectrum(bandFreqsHz, windowSizeSamples, uniform_))); 

        /*
         * Compression
         */
        if(compressionCriterion_ > 0)
        {
            modules_.push_back(unique_ptr<Module>
                    (new CompressSpectrum(compressionCriterion_))); 
        }

        /*
         * Spectral weighting if necessary
         */
        if(weightSpectrum)
        {
            string middleEar = "ANSI";
            string outerEar = "ANSI_FREEFIELD";
            if(hpf_)
                middleEar = "ANSI_HPF";
            if(diffuseField_)
                outerEar = "ANSI_DIFFUSEFIELD";

            modules_.push_back(unique_ptr<Module> 
                    (new WeightSpectrum(middleEar, outerEar))); 
        }

        /*
         * Roex filters
         */
        if(fastBank_)
        {
            modules_.push_back(unique_ptr<Module>
                    (new FastRoexBank(filterSpacing_, interpRoexBank_)));
        }
        else
        {
            modules_.push_back(unique_ptr<Module> 
                    (new RoexBankANSIS342007(1.8, 38.9, filterSpacing_)));
        }
        
        /*
         * Specific loudness
         */
        modules_.push_back(unique_ptr<Module>
                (new SpecificLoudnessGM(ansiSpecificLoudness_)));

        /*
        * Loudness integration 
        */   
        modules_.push_back(unique_ptr<Module> (new
                    IntegratedLoudnessGM(smoothingType_)));

        return 1;
    }
}
