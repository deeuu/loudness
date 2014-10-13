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

    PowerSpectrum::PowerSpectrum(
            const RealVec& bandFreqsHz, 
            const RealVec& windowSizeSecs, 
            bool uniform) :
        Module("PowerSpectrum"),
        bandFreqsHz_(bandFreqsHz),
        windowSizeSecs_(windowSizeSecs),
        uniform_(uniform)
    {}

    PowerSpectrum::~PowerSpectrum()
    { 
        if(initialized_)
        {
            fftw_free(fftInputBuf_);
            fftw_free(fftOutputBuf_);
            LOUDNESS_DEBUG("PowerSpectrum: Buffers destroyed.");

            for(vector<fftw_plan>::iterator i = fftPlans_.begin(); i != fftPlans_.end(); i++)
                fftw_destroy_plan(*i);
            LOUDNESS_DEBUG("PowerSpectrum: Plan(s) destroyed.");
        }
    }

    bool PowerSpectrum::initializeInternal(const SignalBank &input)
    {
        
        //number of windows
        nWindows_ = (int)windowSizeSecs_.size();

        //check input
        if(bandFreqsHz_.size() != (windowSizeSecs_.size()+1))
        {
            //Need to throw an exception, see Debug.h
            LOUDNESS_ERROR(name_ 
                    << ": Number of frequency bands should equal number of windows + 1.");
            return 0;
        }

        //sampling freqeuncy
        int fs = input.getFs();
        
        //window size in samples
        windowSizeSamps_.resize(nWindows_);
        int largestWindowSize = 0;
        for(int i=0; i<nWindows_; i++)
        {
            windowSizeSamps_[i] = round(fs*windowSizeSecs_[i]);
            if(windowSizeSamps_[i]>largestWindowSize)
                largestWindowSize = windowSizeSamps_[i];
            LOUDNESS_DEBUG(name_ <<
                    ": Window size(s) in samples: " 
                    << windowSizeSamps_[i]);
        }

        LOUDNESS_DEBUG(name_ 
                <<": Largest window size in samples: "
                << largestWindowSize);

        //input size must be equal to the largest window
        if(input.getNSamples() != largestWindowSize)
        {
            LOUDNESS_ERROR(name_ << ": Number of input samples: " 
                    << input.getNSamples() 
                    << " but must be equal to: " 
                    << largestWindowSize);
            return 0;
        }

        //create N windows and associated fft plans
        windows_.resize(nWindows_);
        fftSize_.resize(nWindows_, 0);
    
        //allocate memory for FFT input buffers...all FFT inputs can make use of a single buffer
        //since we are not doing zero phase insersion
        fftSize_[0] = pow(2, ceil(log2(largestWindowSize)));
        fftInputBuf_ = (Real*) fftw_malloc(sizeof(Real) * fftSize_[0]);
        fftOutputBuf_ = (Real*) fftw_malloc(sizeof(Real) * fftSize_[0]);
        
        //only 1 plan required if uniform spectral sampling 
        LOUDNESS_DEBUG(name_
                << ": FFT buffer sizes: " 
                << fftSize_[0] 
                << ", memory allocated.");

        if(uniform_)
        {
            fftPlans_.push_back(fftw_plan_r2r_1d(fftSize_[0],
                        fftInputBuf_, fftOutputBuf_, FFTW_R2HC, FFTW_PATIENT));
            LOUDNESS_DEBUG(name_ <<
                    ": Created a single " 
                    << fftSize_[0] 
                    << "-point FFT plan for uniform spectral sampling.");
        }
        else
            LOUDNESS_DEBUG(name_ << ": Creating multiple plans...");

        //create windows and multiple plans (if needed)
        for(int i=0; i<nWindows_; i++)
        {
            if(!uniform_)
            {
                fftSize_[i] = pow(2,ceil(log2(windowSizeSamps_[i])));
                fftPlans_.push_back(fftw_plan_r2r_1d(fftSize_[i],
                            fftInputBuf_, fftOutputBuf_, FFTW_R2HC, FFTW_PATIENT));
            }
            else
                fftSize_[i] = fftSize_[0];

            //windows
            windows_[i].assign(windowSizeSamps_[i], 0.0);
            hannWindow(windows_[i], fftSize_[i]);
            LOUDNESS_DEBUG(name_ <<
                    ": Using window size " << windowSizeSamps_[i]
                    << ", for "  <<  fftSize_[i] << "-point FFT");
        }

        //desired bins indices (lo and hi) per band
        bandBinIndices_.resize(nWindows_);

        //appropriate delay for temporal alignment
        temporalCentre_ = (largestWindowSize-1)/2.0;
        LOUDNESS_DEBUG(name_ 
                <<": Temporal centre of largest window: " 
                << temporalCentre_);

        //the rest
        windowDelay_.resize(nWindows_);
        for(int i=0; i<nWindows_; i++)
        {
            //SignalBank offset for temporal alignment
            Real tc2 = (windowSizeSamps_[i]-1)/2.0;
            windowDelay_[i] = (int)round(temporalCentre_ - tc2);

            LOUDNESS_DEBUG(name_ 
                    << ": Window of size " << windowSizeSamps_[i] 
                    << " samples requires a " << windowDelay_[i] 
                    << " sample delay."
                    << " Centre point (samples): " 
                    << tc2+windowDelay_[i]);
            
            //bin indices to use for compiled spectrum
            bandBinIndices_[i].resize(2);
            //These are NOT the nearest components but satisfies f_k in [f_lo, f_hi)
            bandBinIndices_[i][0] = ceil(bandFreqsHz_[i]*fftSize_[i]/fs);
            bandBinIndices_[i][1] = ceil(bandFreqsHz_[i+1]*fftSize_[i]/fs)-1;

            //for (f_lo, f_hi] use this line:
            //bandBinIndices_[i][1] = floor(bandFreqsHz_[i+1]*fftSize_[i]/fs);

            //exclude DC and Nyquist if found
            if(bandBinIndices_[i][0]==0)
            {
                LOUDNESS_WARNING("PowerSpectrum: DC found...excluding.");
                bandBinIndices_[i][0] = 1;
            }
            if(bandBinIndices_[i][1] >= (fftSize_[i]/2.0))
            {
                LOUDNESS_WARNING("PowerSpectrum: Bin is >= nyquist...excluding.");
                bandBinIndices_[i][1] = (ceil(fftSize_[i]/2.0)-1);
            }
        }

        //count total number of bins and ensure no overlap
        int nBins = 0;
        for(int i=1; i<nWindows_; i++)
        {
            while((bandBinIndices_[i][0]*fs/fftSize_[i]) <= (bandBinIndices_[i-1][1]*fs/fftSize_[i-1]))
                bandBinIndices_[i][0] += 1;

            //this line will alter the band frequencies slightly to ensure closely spaced bins
            /*
             while(((bandBinIndices_[i-1][1]+1)*fs/fftSize_[i-1]) < (bandBinIndices_[i][0]*fs/fftSize_[i]))
                bandBinIndices_[i-1][1] += 1;   
            */
            nBins += bandBinIndices_[i-1][1]-bandBinIndices_[i-1][0] + 1;
        }
        
        //total number of bins in the output spectrum
        nBins += bandBinIndices_[nWindows_-1][1]-bandBinIndices_[nWindows_-1][0] + 1;

        LOUDNESS_DEBUG(name_ 
                << ": Total number of bins comprising the output spectrum: " << nBins);

        #if defined(DEBUG)
        for(int i=0; i<nWindows_; i++)
        {
            Real edgeLo = bandBinIndices_[i][0]*fs/(float)fftSize_[i];
            Real edgehi = bandBinIndices_[i][1]*fs/(float)fftSize_[i];
            LOUDNESS_DEBUG(name_ 
                    << ": Band edges (Hz) for window of size: " 
                    << windowSizeSamps_[i] << " = [ " 
                    << std::setprecision (7) 
                    << edgeLo << ", " 
                    <<  edgehi 
                    << " ].");
        }
        #endif 

        //initialize the output SignalBank
        output_.initialize(nBins, 1, fs);
        output_.setFrameRate(input.getFrameRate());

        //output frequencies in Hz
        int j = 0, k = 0;
        for(int i=0; i<nWindows_; i++)
        {
            j = bandBinIndices_[i][0];
            while(j <= bandBinIndices_[i][1])
                output_.setCentreFreq(k++, (j++)*fs/(Real)fftSize_[i]);
        }

        return 1;
    }

    void PowerSpectrum::processInternal(const SignalBank &input)
    {
        //for each window
        int binWriteIdx = 0;
        for(int i=0; i<nWindows_; i++)
        {
            //fill the buffer
            for(int j=0; j<windowSizeSamps_[i]; j++)
                fftInputBuf_[j] = input.getSample(0, windowDelay_[i]+j)*windows_[i][j];

            //compute fft
            if(uniform_)
                fftw_execute(fftPlans_[0]);
            else
                fftw_execute(fftPlans_[i]);

            //clear windowed data
            for(int j=0; j<windowSizeSamps_[i]; j++)
                fftInputBuf_[j] = 0.0;

            //Extract components from band and compute powers
            Real re, im;
            for(int j=bandBinIndices_[i][0]; j<=bandBinIndices_[i][1]; j++)
            {
                re = fftOutputBuf_[j];
                im = fftOutputBuf_[fftSize_[i]-j];
                output_.setSample(binWriteIdx++, 0, re*re + im*im);
            }
        }
    }

    void PowerSpectrum::hannWindow(RealVec &w, int fftSize)
    {
        int windowSize = (int)w.size();
        Real norm = sqrt(2.0/(fftSize*windowSize*0.375*2e-5*2e-5));

        for(int i=0; i< windowSize; i++)
            w[i] = norm*(0.5+0.5*cos(2*PI*(i-0.5*(windowSize-1))/windowSize));
    }

    void PowerSpectrum::resetInternal()
    {
        for(int i=0; i<nWindows_; i++)
            windowDelay_[i] = (int)round(temporalCentre_ - (windowSizeSamps_[i]-1)/2.0);
    }
}

