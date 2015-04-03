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

#include "FFT.h"

namespace loudness{

    FFT::FFT(int fftSize) :
        fftSize_(fftSize),
        nReals_(0),
        nImags_(0),
        initialized_(false)
    {
        LOUDNESS_DEBUG("FFT: Constructed");
    }

    FFT::~FFT()
    { 
        freeFFTW();
    }

    void FFT::freeFFTW(){

        if (initialized_)
        {
            fftw_free(fftInputBuf_);
            fftw_free(fftOutputBuf_);
            LOUDNESS_DEBUG("FFT: Buffers destroyed.");

            fftw_destroy_plan(fftPlan_);
            LOUDNESS_DEBUG("FFT: Plan destroyed.");
        }
    }

    bool FFT::initialize()
    {
        LOUDNESS_DEBUG("FFT: Setting up...");
        freeFFTW();
        //don't worry, we are protected from reinitialisation
        //allocate memory for FFT input buffers...all FFT inputs can make use of a single buffer
        //since we are not doing zero phase insersion
        fftInputBuf_ = (Real*) fftw_malloc(sizeof(Real) * fftSize_);
        fftOutputBuf_ = (Real*) fftw_malloc(sizeof(Real) * fftSize_);
        LOUDNESS_DEBUG("FFT: Allocated input and output buffers for an FFT size of " << fftSize_);
        
        fftPlan_ = fftw_plan_r2r_1d(fftSize_, fftInputBuf_,
                fftOutputBuf_, FFTW_R2HC, FFTW_PATIENT);

        LOUDNESS_DEBUG("FFT: Plan set up");

        //number of positive output components
        //see: http://www.fftw.org/fftw2_doc/fftw_3.html 
        nPositiveComponents_ = fftSize_/2 + 1;
        nReals_ = nPositiveComponents_;
        nImags_ = nReals_ - 2 + (fftSize_ % 2);

        LOUDNESS_DEBUG("FFT: Number of positive frequency components = " << nPositiveComponents_);

        initialized_ = true;
        
        return 1;
    }

    void FFT::process(const Real* input, int length)
    {
        if(initialized_)
        {
            //fill the buffer
            int i = fftSize_;
            while(i > length)
                fftInputBuf_[--i] = 0.0;
            while(i > 0)
                fftInputBuf_[--i] = input[i];

            //compute fft
            fftw_execute(fftPlan_);
        }
    }

    int FFT::getFftSize() const
    {
        return fftSize_;
    }

    int FFT::getNPositiveComponents() const
    {
        return nPositiveComponents_;
    }

}
