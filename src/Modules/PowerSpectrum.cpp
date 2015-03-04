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
        nWindows_ = input.getNChannels();

        if((int)bandFreqsHz_.size() != (nWindows_ + 1))
        {
            LOUDNESS_ERROR(name_ 
                    << ": Number of frequency bands should equal number of windows + 1.");
            return 0;
        }

        //sampling freqeuncy
        int fs = input.getFs();
        
        //temporary vectors
        int windowSizePrev = input.getNSamples();//largest window
        vector<int> windowSize(nWindows_, windowSizePrev);
        vector<int> fftSize(nWindows_, pow(2, ceil(log2(windowSizePrev))));
        //work out FFT sizes (force power of 2)
        for(int w=1; w<nWindows_; w++)
        {
            const vector<Real> &signal = input.getSignal(w);
            windowSize[w] = (int)signal.size();

            if(!uniform_)
                fftSize[w] = pow(2, ceil(log2(windowSize[w])));

            //windows must be largest to smallest
            if (windowSize[w] > windowSizePrev)
            {
                LOUDNESS_ERROR(name_ << ": Window lengths must be in descending order.");
                return 0;
            }

            windowSizePrev = windowSize[w];
        }
        //One FFT for all windows - uniform spectral sampling
        if(uniform_)
        {
            ffts_.push_back(unique_ptr<FFT> (new FFT(fftSize[0]))); 
            ffts_[0] -> initialize();
        }
        else //seperate
        {
            for(int w=0; w<nWindows_; w++)
            {
                ffts_.push_back(unique_ptr<FFT> (new FFT(fftSize[w]))); 
                ffts_[w] -> initialize();
            }
        }

        //desired bins indices (lo and hi) per band
        bandBinIndices_.resize(nWindows_);
        normFactor_.resize(nWindows_);
        for(int i=0; i<nWindows_; i++)
        {
            //bin indices to use for compiled spectrum
            bandBinIndices_[i].resize(2);
            //These are NOT the nearest components but satisfies f_k in [f_lo, f_hi)
            bandBinIndices_[i][0] = ceil(bandFreqsHz_[i]*fftSize[i]/fs);
            LOUDNESS_DEBUG(name_ << ": band low : " << bandBinIndices_[i][0]);
            bandBinIndices_[i][1] = ceil(bandFreqsHz_[i+1]*fftSize[i]/fs)-1;
            LOUDNESS_DEBUG(name_ << ": band hi : " << bandBinIndices_[i][1]);

            if(bandBinIndices_[i][1]==0)
            {
                LOUDNESS_ERROR(name_ << ": No components found in band number " << i);
                return 0;
            }

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
                normFactor_[i] = 2.0/(fftSize[i] * windowSize[i]);
            else
                normFactor_[i] = 2.0/fftSize[i];
            LOUDNESS_DEBUG(name_ << ": Normalisation factor : " << normFactor_[i]);
        }

        //count total number of bins and ensure no overlap
        int nBins = 0;
        for(int i=1; i<nWindows_; i++)
        {
            while((bandBinIndices_[i][0]*fs/fftSize[i]) <= (bandBinIndices_[i-1][1]*fs/fftSize[i-1]))
                bandBinIndices_[i][0] += 1;
            nBins += bandBinIndices_[i-1][1]-bandBinIndices_[i-1][0] + 1;
        }
        
        //total number of bins in the output spectrum
        nBins += bandBinIndices_[nWindows_-1][1]-bandBinIndices_[nWindows_-1][0] + 1;
        LOUDNESS_DEBUG(name_ 
                << ": Total number of bins comprising the output spectrum: " << nBins);

        //initialize the output SignalBank
        output_.initialize(nBins, 1, fs);
        output_.setFrameRate(input.getFrameRate());

        //output frequencies in Hz
        int j = 0, k = 0;
        for(int i=0; i<nWindows_; i++)
        {
            j = bandBinIndices_[i][0];
            while(j <= bandBinIndices_[i][1])
                output_.setCentreFreq(k++, (j++)*fs/(Real)fftSize[i]);
        }

        return 1;
    }

    void PowerSpectrum::processInternal(const SignalBank &input)
    {
        //for each window
        int fftIdx = 0, writeIdx = 0;
        for(int i=0; i<nWindows_; i++)
        {
            if(!uniform_)
                fftIdx = i;
            
            //Do the FFT
            ffts_[fftIdx] -> process(input.getSignal(i));

            //Extract components from band and compute powers
            Real re, im, power;
            for(int j=bandBinIndices_[i][0]; j<=bandBinIndices_[i][1]; j++)
            {
                re = ffts_[fftIdx] -> getReal(j);
                im = ffts_[fftIdx] -> getImag(j);
                power = normFactor_[i] * (re*re + im*im);
                output_.setSample(writeIdx++, 0, power);
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
