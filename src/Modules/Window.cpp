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
        periodic_(periodic),
        normalisation_("energy"),
        ref_(2e-5),
        sum_(false),
        average_(false),
        squareInput_(false),
        sqrRoot_(false),
        alignOutput_(false)
    {}
    Window::Window(const string &windowType, int length, bool periodic) :
        Module("Window"),
        windowType_(windowType),
        periodic_(periodic),
        normalisation_("energy"),
        ref_(2e-5),
        sum_(false),
        average_(false),
        squareInput_(false),
        sqrRoot_(false),
        alignOutput_(false)
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

    bool Window::normaliseWindow(RealVec &window, const string &normalisation, double ref)
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
            if (normalisation == "energy")
            {
                normFactor = sqrt(wSize/sumSquares);
            }
            else if (normalisation == "amplitude")
            {
                normFactor = wSize/sum;
            }
            else 
            {
                LOUDNESS_WARNING(name_ << ": Normalisation must be 'energy' or 'amplitude'");
                return 0;
            }
            normFactor /= ref;
            for(uint i=0; i < wSize; i++)
                window[i] *= normFactor;
        }
        return 1;
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

        //first window (largest) does not require a shift
        windowOffset_.push_back(0);
        //check if we are using multi windows on one input channel
        if((input.getNChannels()==1) && (nWindows_>1))
        {
            LOUDNESS_DEBUG(name_ << ": Using parallel windows");
            parallelWindows_ = true;
            int alignmentSample = ceil((largestWindowSize_-1)/2.0);
            LOUDNESS_DEBUG(name_ << ": Alignment sample = " << alignmentSample);
            for(int w=1; w<nWindows_; w++)
            {
                int thisCentreSample = ceil((length_[w]-1)/2.0);
                int thisWindowOffset = alignmentSample - thisCentreSample;
                windowOffset_.push_back(thisWindowOffset);
                LOUDNESS_DEBUG(name_ << ": Centre sample for window " << w << " = " << thisCentreSample);
                LOUDNESS_DEBUG(name_ << ": Offset for window " << w << " = " << thisWindowOffset);
            }
        }
        //generate the normalised window functions
        for (int w=0; w<nWindows_; w++)
        {
            window_[w].assign(length_[w],0.0);
            generateWindow(window_[w], windowType_, periodic_);
            normaliseWindow(window_[w], normalisation_, ref_);
            LOUDNESS_DEBUG(name_ << ": Length of window " << w << " = " << window_[w].size());
        }
        //initialise the output signal
        if(sum_)
            output_.initialize(nWindows_, 1, input.getFs());//frame rate?
        else
        {
            output_.initialize(nWindows_, largestWindowSize_, input.getFs());
            if(!alignOutput_)
            {
                //we resize the outputs so PowerSpectrum knows
                for(int w=0; w < nWindows_; w++)
                    output_.resizeSignal(w, length_[w]);
            }
        }
        output_.setFrameRate(input.getFrameRate());

        return 1;
    }

    void Window::processInternal(const SignalBank &input)
    {
        int chnIdx=0, offset=0, offset2=0;
        for(int ear=0; ear<input.getNEars(); ear++)
        {
            //if output windows are not aligned, can keep incrementing the
            //output array and move through it's dimensions.
            Real* outputSignal = output_.getSignal(0);
            for(int w=0; w<nWindows_; w++)
            {
                const Real* inputSignal = input.getSignal(ear, 0, windowOffset_[w]);

                //if aligned, better get the correct index.
                if(alignOutput_)
                    outputSignal = output_.getSignal(ear, 0, windowOffset_[w]);

                for(int smp=0; smp<length_[w]; smp++)
                    *outputSignal++ = window[w][smp] * (*inputSignal++);
            }
        }
    }

    void Window::resetInternal()
    {
    }

    void Window::setRef(const Real ref)
    {
        ref_ = ref;
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

    bool Window::getSum() const
    {
        return sum_;
    }

    bool Window::getSqrRoot() const
    {
        return sqrRoot_;
    }
}
