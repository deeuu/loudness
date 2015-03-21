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

#include "PowerSpectrum.h"

namespace loudness{

    PowerSpectrum::PowerSpectrum(const RealVec& bandFreqsHz, bool uniform):
        Module("PowerSpectrum"),
        bandFreqsHz_(bandFreqsHz),
        uniform_(uniform),
        normalisation_("averageEnergy")
    {}

    PowerSpectrum::~PowerSpectrum()
    {}

    bool PowerSpectrum::initializeInternal(const SignalBank &input)
    {
        
        //number of windows
        int nWindows = input.getNChannels();

        //If passed in by Window module, the window lengths are given here
        windowSize_ = input.getEffectiveSignalLengths();

        //Fix conditions where this module is used independently of Window.cpp
        if((int)windowSize_.size() != nWindows)
        {
            windowSize_.assign(nWindows, input.getNSamples());
            uniform_ = true;
        }

        //some checks
        LOUDNESS_ASSERT((int)bandFreqsHz_.size() == (nWindows + 1),
                name_ << ": Number of frequency bands should equal number of input channels + 1.");
        LOUDNESS_ASSERT(!anyAscendingValues(windowSize_),
                    name_ << ": Window lengths must be in descending order.");

        //work out FFT configuration (constrain to power of 2)
        int largestWindowSize = input.getNSamples();
        vector<int> fftSize(nWindows, nextPowerOfTwo(largestWindowSize));
        if(uniform_)
        {
            ffts_.push_back(unique_ptr<FFT> (new FFT(fftSize[0]))); 
            ffts_[0] -> initialize();
        }
        else
        {
            for(int w=0; w<nWindows; w++)
            {
                fftSize[w] = nextPowerOfTwo(windowSize_[w]);
                ffts_.push_back(unique_ptr<FFT> (new FFT(fftSize[w]))); 
                ffts_[w] -> initialize();
            }
        }

        //desired bins indices (lo and hi) per band
        bandBinIndices_.resize(nWindows);
        normFactor_.resize(nWindows);
        int fs = input.getFs();
        for(int i=0; i<nWindows; i++)
        {
            //bin indices to use for compiled spectrum
            bandBinIndices_[i].resize(2);
            //These are NOT the nearest components but satisfies f_k in [f_lo, f_hi)
            bandBinIndices_[i][0] = ceil(bandFreqsHz_[i]*fftSize[i]/fs);
            LOUDNESS_DEBUG(name_ << ": band low : " << bandBinIndices_[i][0]);
            bandBinIndices_[i][1] = ceil(bandFreqsHz_[i+1]*fftSize[i]/fs)-1;
            LOUDNESS_DEBUG(name_ << ": band hi : " << bandBinIndices_[i][1]);

            LOUDNESS_ASSERT(bandBinIndices_[i][1]>0,
                    name_ << ": No components found in band number " << i);

            //exclude DC and Nyquist if found
            int nyqIdx = (fftSize[i]/2) + (fftSize[i]%2);
            if(bandBinIndices_[i][0]==0)
            {
                LOUDNESS_WARNING(name_ << ": DC found...excluding.");
                bandBinIndices_[i][0] = 1;
            }
            if(bandBinIndices_[i][1] >= nyqIdx)
            {
                LOUDNESS_WARNING(name_ << 
                        ": Bin is >= nyquist...excluding.");
                bandBinIndices_[i][1] = nyqIdx-1;
            }

            //Power spectrum normalisation
            if(normalisation_ == "averageEnergy")
                normFactor_[i] = 2.0/(fftSize[i] * windowSize_[i]);
            else
                normFactor_[i] = 2.0/fftSize[i];
            LOUDNESS_DEBUG(name_ << ": Normalisation factor : " << normFactor_[i]);
        }

        //count total number of bins and ensure no overlap
        int nBins = 0;
        for(int i=1; i<nWindows; i++)
        {
            while((bandBinIndices_[i][0]*fs/fftSize[i]) <= (bandBinIndices_[i-1][1]*fs/fftSize[i-1]))
                bandBinIndices_[i][0] += 1;
            nBins += bandBinIndices_[i-1][1]-bandBinIndices_[i-1][0] + 1;
        }
        
        //total number of bins in the output spectrum
        nBins += bandBinIndices_[nWindows-1][1]-bandBinIndices_[nWindows-1][0] + 1;
        LOUDNESS_DEBUG(name_ 
                << ": Total number of bins comprising the output spectrum: " << nBins);

        //initialize the output SignalBank
        output_.initialize(input.getNEars(), nBins, 1, fs);
        output_.setFrameRate(input.getFrameRate());

        //output frequencies in Hz
        int j = 0, k = 0;
        for(int i=0; i<nWindows; i++)
        {
            j = bandBinIndices_[i][0];
            while(j <= bandBinIndices_[i][1])
                output_.setCentreFreq(k++, (j++)*fs/(Real)fftSize[i]);
        }

        return 1;
    }

    void PowerSpectrum::processInternal(const SignalBank &input)
    {
        int fftIdx = 0;
        int nWindows = windowSize_.size();
        for(int ear=0; ear<input.getNEars(); ear++)
        {
            //get a single sample pointer for moving through channels
            Real* outputSignal = output_.getSingleSampleWritePointer(ear,0);

            for(int chn=0; chn<nWindows; chn++)
            {
                if(!uniform_)
                    fftIdx = chn;

                //Do the FFT
                ffts_[fftIdx] -> process(input.getSignalReadPointer(ear, chn, 0), windowSize_[chn]);

                //Extract components from band and compute powers
                Real re, im;
                int bin = bandBinIndices_[chn][0];
                while(bin <= bandBinIndices_[chn][1])
                {
                    re = ffts_[fftIdx] -> getReal(bin);
                    im = ffts_[fftIdx] -> getImag(bin++);
                    *outputSignal++ = normFactor_[chn] * (re*re + im*im);
                }
            }
        }
    }

    void PowerSpectrum::resetInternal()
    {}

    void PowerSpectrum::setNormalisation(const string &normalisation)
    {
        normalisation_ = normalisation;
    }
}
