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

#include "Window.h"

namespace loudness{

    Window::Window(const string &windowType, const IntVec &length, bool periodic) :
        Module("Window"),
        windowType_(windowType),
        length_(length),
        periodic_(periodic)
    {}
    Window::Window(const string &windowType, int length, bool periodic) :
        Module("Window"),
        windowType_(windowType),
        periodic_(periodic)
    {
       length_.assign(1, length); 
    }
    Window::Window():
        Module("Window")
    {}
    
    Window::~Window()
    {
    }
    
    void Window::generateWindow(RealVec &window, const string &windowType, bool periodic)
    {
        if(windowType == "hann")
            hann(window, periodic);
    }

    void Window::setNormalisation(const string &normalisation)
    {
        normalisation_ = normalisation;
    }

    void Window::normaliseWindow(RealVec &window, const string &normalisation)
    {
        if(!normalisation.empty())
        {
            double x = 0.0;
            double sum = 0.0, sumSquares = 0.0;
            double normFactor = 1.0;
            uint wSize = window.size();
            for(uint i=0; i < wSize; i++)
            {
                 x = window[i];
                 sum += x;
                 sumSquares += x*x;
            }
            if (normalisation == "rms")
                normFactor = 1.0/sqrt(sumSquares/wSize);
            else if (normalisation == "amplitude")
                normFactor = 1.0 / sum;
            for(uint i=0; i < wSize; i++)
                window[i] *= normFactor;
        }
    }
    
    void Window::hann(RealVec &window, bool periodic)
    {
        unsigned int N = window.size();
        int denom = N-1;//produces zeros on both sides
        if(periodic)//Harris (1978) Eq 27b 
            denom = N;
        for(unsigned int i=0; i<window.size(); i++)
        {
            window[i] = 0.5 - 0.5 * cos(2.0*PI*i/denom);
        }
    }

    bool Window::initializeInternal(const SignalBank &input)
    {
        if(input.getNSamples() != length_[0])
        {
            LOUDNESS_ERROR(name_ << ": Number of samples is not equal to the largest window size!");
            return 0;
        }

        //number of windows
        nWindows_ = (int)length_.size();
        LOUDNESS_DEBUG(name_ << ": Number of windows = " << nWindows_);
        window_.resize(nWindows_);
        largestWindowSize_ = length_[0];
        LOUDNESS_DEBUG(name_ << ": Largest window size = " << largestWindowSize_);
        
        //check if we are using multi windows on one input channel
        if((input.getNChannels()==1) && (nWindows_>1))
        {
            LOUDNESS_DEBUG(name_ << ": Using parallel windows");
            parallelWindows_ = true;
            int alignmentSample = ceil((largestWindowSize_-1)/2.0);
            LOUDNESS_DEBUG(name_ << ": Alignment sample = " << alignmentSample);
            windowOffset_.push_back(0);
            for(int w=1; w<nWindows_; w++)
            {
                int thisCentreSample = ceil((length_[w]-1)/2.0);
                int thisWindowOffset = alignmentSample - thisCentreSample;
                windowOffset_.push_back(thisWindowOffset);
                LOUDNESS_DEBUG(name_ << ": Centre sample for window " << w << " = " << thisCentreSample);
                LOUDNESS_DEBUG(name_ << ": Offset for window " << w << " = " << thisWindowOffset);
            }
        }
        for (int w=0; w<nWindows_; w++)
        {
            window_[w].assign(length_[w],0.0);
            generateWindow(window_[w], windowType_, periodic_);
            normaliseWindow(window_[w], normalisation_);
            LOUDNESS_DEBUG(name_ << ": Length of window " << w << " = " << window_[w].size());
        }
        //initialise the output signal
        if(sum_)
            output_.initialize(nWindows_, 1, input.getFs());//frame rate?
        else
            output_.initialize(nWindows_, largestWindowSize_, input.getFs());

        return 1;
    }

    void Window::processInternal(const SignalBank &input)
    {
        int chnIdx, offset1, offset2;
        for(int w=0; w<nWindows_; w++)
        {
            //offset 1 is the read position
            offset1 = windowOffset_[w];
            //offset 2 is the write position
            offset2 = 0;
            if(alignOutput_)
                offset2 = offset1;
            if(parallelWindows_)
                chnIdx = 0;

            if(sum_)
            {
                double x=0.0, y=0.0;
                if(squareInput_)
                {
                    for (int smp=0; smp<length_[w]; smp++)
                    {
                        x = input.getSample(chnIdx, offset1+smp);
                        x *= x;
                        y += x;
                    }
                    if(sqrRoot_)
                        y = sqrt(x);
                }
                else
                {    
                    for (int smp=0; smp<length_[w]; smp++)
                        y += window_[w][smp] * input.getSample(chnIdx, offset1+smp);
                }
                output_.setSample(w, 0, y);
            }
            else //normal windowing e.g. for FFT
            {
                for (int smp=0; smp<length_[w]; smp++)
                    output_.setSample(w, offset2+smp, window_[w][smp] * input.getSample(chnIdx, offset1+smp));
            }
        }
    }

    void Window::resetInternal()
    {
    }

    void Window::setAlignOutput(bool alignOutput)
    {
        alignOutput_ = alignOutput;
    }
 
    void Window::setSqrRoot(bool sqrRoot)
    {
        sqrRoot_ = sqrRoot;
    }

    void Window::setSum(bool sum)
    {
        sum_ = sum;
    }
    void Window::setSquareInput(bool squareInput)
    {
        squareInput_ = squareInput;
    }

    bool Window::getSquareInput() const
    {
        return squareInput_;
    }
}
