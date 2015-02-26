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

    FFT::FFT(int fftSize, const string & returnType) :
        Module("FFT"),
        fftSize_(fftSize),
        returnType_(returnType)
    {}

    FFT::~FFT()
    { 
        if(initialized_) //I thought you said don't worry
        {
            fftw_free(fftInputBuf_);
            fftw_free(fftOutputBuf_);
            LOUDNESS_DEBUG("FFT: Buffers destroyed.");

            for(vector<fftw_plan>::iterator i = fftPlans_.begin(); i != fftPlans_.end(); i++)
                fftw_destroy_plan(*i);
            LOUDNESS_DEBUG("FFT: Plan destroyed.");
        }
    }

    bool FFT::initialize(const SignalBank &input)
    {
        if(input.getNSamples() > fftSize_)
        {
            LOUDNESS_ERROR(name_ << ": Number of input samples is > FFT size");
            return 0;
        }
        else
        {
            //don't worry, we are protected from reinitialisation
            //allocate memory for FFT input buffers...all FFT inputs can make use of a single buffer
            //since we are not doing zero phase insersion
            fftInputBuf_ = (Real*) fftw_malloc(sizeof(Real) * fftSize_);
            fftOutputBuf_ = (Real*) fftw_malloc(sizeof(Real) * fftSize_);
            
            fftPlan_ = fftw_plan_r2r_1d(fftSize_, fftInputBuf_,
                    fftOutputBuf_, FFTW_R2HC, FFTW_PATIENT);

            //FFT is viewed as a filter bank here so we go column wise
            //Not as efficient due to contiguous memory though
            if (returnType_ == "complex")
                returnTypeInt_ = 0;
            if (returnType_ == "power")
                returnTypeInt_ = 1;
            if (returnType_ == "magnitude")
                returnTypeInt_ = 2;
                output_.initialize(fftSize_, 2, input.getFs());
            else
                output_.initialize(fftSize_, 1, input.getFs());

            return 1;
        }
    }

    void FFT::processInternal(const SignalBank &input)
    {
        //fill the buffer
        for(int i=0; j<input.getNSamples(); i++)
            fftInputBuf_[j] = input.getSample(0, i);

        //compute fft
        fftw_execute(fftPlan_);

        //clear for next one
        double re, im;
        for(int j=0; j<fftSize_; j++)
        {
            fftInputBuf_[j] = 0.0;
            re = fftOutputBuf_[j];
            im = fftOutputBuf_[fftSize_ - j];
            if(returnTypeInt_ == 0)
            {
                output_.setSample(j, 0, re);
                output_.setSample(j, 1, im);
            }
            else
            {
                y = re*re + im*im;
                if(returnTypeInt_ == 2)
                    y = sqrt(y);
                output_.setSample(j, 0, y);
            }
        }
    }

    void FFT::resetInternal()
    {}
}

