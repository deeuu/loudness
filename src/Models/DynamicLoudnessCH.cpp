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

#include "../cnpy/cnpy.h"
#include "../Support/AuditoryTools.h"
#include "../Modules/FrameGenerator.h"
#include "../Modules/FIR.h"
#include "../Modules/IIR.h"
#include "../Modules/Window.h"
#include "../Modules/PowerSpectrum.h"
#include "../Modules/GoertzelPS.h"
#include "../Modules/CompressSpectrum.h"
#include "../Modules/WeightSpectrum.h"
#include "../Modules/DoubleRoexBank.h"
#include "../Modules/IntegratedLoudnessGM.h"
#include "DynamicLoudnessCH.h"

namespace loudness{

    DynamicLoudnessCH::DynamicLoudnessCH(string pathToFilterCoefs) :
        Model("DynamicLoudnessCH", true)
    {
        loadParameterSet(FASTER1);
    }
    

    DynamicLoudnessCH::~DynamicLoudnessCH()
    {
    }

    void DynamicLoudnessCH::setDiffuseField(bool diffuseField)
    {
        diffuseField_ = diffuseField;
    }
    void DynamicLoudnessCH::setGoertzel(bool goertzel)
    {
        goertzel_ = goertzel;
    }
    void DynamicLoudnessCH::setDiotic(bool diotic)
    {
        diotic_ = diotic;
    }
    void DynamicLoudnessCH::setUniform(bool uniform)
    {
        uniform_ = uniform;
    }

    void DynamicLoudnessCH::setFilterSpacing(Real filterSpacing)
    {
        filterSpacing_ = filterSpacing;
    }
    void DynamicLoudnessCH::setCompressionCriterion(Real compressionCriterion)
    {
        compressionCriterion_ = compressionCriterion;
    }

    void DynamicLoudnessCH::loadParameterSet(ParameterSet set)
    {
        //common to all
        setTimeStep(0.001);
        setDiffuseField(false);
        setGoertzel(false);
        setDiotic(true);
        setUniform(true);
        setFilterSpacing(0.1);
        setCompressionCriterion(0.0);

        switch(set){
            case CH02:
                break;
            case FASTER1:
                setFilterSpacing(0.25);
                setCompressionCriterion(0.3);
                break;
        }
    }

    bool DynamicLoudnessCH::initializeInternal(const SignalBank &input)
    {
        //if filter coefficients have not been provided
        //use spectral weighting to approximate outer and middle ear response
        if(!pathToFilterCoefs_.empty())
        {
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
         * Frame generator for spectrogram
         */
        int windowSize = round(0.128*input.getFs());
        if(!goertzel_)
        {
            int hopSize = round(timeStep_*input.getFs());
            modules_.push_back(unique_ptr<Module> 
                    (new FrameGenerator(windowSize, hopSize, false)));
        }

        /*
         * Multi-resolution spectrogram
         */
        RealVec bandFreqsHz {10, 80, 500, 1250, 2540, 4050, 16001};

        //window spec
        RealVec windowSizeSecs {0.128, 0.064, 0.032, 0.016, 0.008, 0.004};
        vector<int> windowSizeSamples(6,0);
        //round to nearest sample and force to be even such that centre samples
        //are aligned.
        for(int w=0; w<6; w++)
        {
            windowSizeSamples[w] = (int)round(windowSizeSecs[w] * input.getFs());
            windowSizeSamples[w] += windowSizeSamples[w]%2;
        }
        
        //create module
        if(goertzel_)
        {
            modules_.push_back(unique_ptr<Module> 
                    (new GoertzelPS(bandFreqsHz, windowSizeSecs, timeStep_))); 
        }
        else{
            //configure windowing
            modules_.push_back(unique_ptr<Module>
                    (new Window("hann", windowSizeSamples, true)));
            //power spectrum
            modules_.push_back(unique_ptr<Module> 
                    (new PowerSpectrum(bandFreqsHz, uniform_))); 
        }

        /*
         * Compression
         */
        if(compressionCriterion_ > 0)
        {
            if(goertzel_)
            {
                LOUDNESS_WARNING(name_ << ": Compression cannot be used with bank of Goertzels");
            }
            else
            {
                modules_.push_back(unique_ptr<Module>
                        (new CompressSpectrum(compressionCriterion_))); 
            }
        }

        /*
         * Spectral weighting
         */
        if(pathToFilterCoefs_.empty())
        {
            string middleEar = "CHEN_ETAL";
            string outerEar = "ANSI_FREEFIELD";
            if(diffuseField_)
                outerEar = "ANSI_DIFFUSEFIELD";

            modules_.push_back(unique_ptr<Module> 
                    (new WeightSpectrum(middleEar, outerEar))); 
        }

        /*
         * Roex filters
         */
        modules_.push_back(unique_ptr<Module> (new DoubleRoexBank(1.5, 40.2,
                        filterSpacing_)));
        
        /*
        * Loudness integration 
        */   
        modules_.push_back(unique_ptr<Module> (new
                    IntegratedLoudnessGM(IntegratedLoudnessGM::CH12, diotic_,
                        1.53e-8)));

        return 1;
    }

}

