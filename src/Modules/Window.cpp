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

    Window::Window(const string &windowType, const IntVec &length, bool periodic, bool alignOutput) :
        Module("Window"),
        windowType_(windowType),
        length_(length),
        periodic_(periodic),
        alignOutput_(alignOutput),
        normalisation_("energy"),
        ref_(2e-5)
    {}
    Window::Window(const string &windowType, int length, bool periodic) :
        Module("Window"),
        windowType_(windowType),
        periodic_(periodic),
        alignOutput_(false),
        normalisation_("energy"),
        ref_(2e-5)
    {
       length_.assign(1, length); 
    }
    Window::Window():
        Module("Window")
    {}
    
    Window::~Window()
    {
    }
    
   
    bool Window::initializeInternal(const SignalBank &input)
    {
        LOUDNESS_ASSERT(input.getNSamples() == length_[0], 
                    name_ << ": Number of input samples does not equal the largest window size!");

        //number of windows
        nWindows_ = (int)length_.size();
        LOUDNESS_DEBUG(name_ << ": Number of windows = " << nWindows_);
        window_.resize(nWindows_);

        //Largest window should be the first
        largestWindowSize_ = length_[0];
        LOUDNESS_DEBUG(name_ << ": Largest window size = " << largestWindowSize_);

        //first window (largest) does not require a shift
        windowOffset_.push_back(0);

        //check if we are using multi windows on one input channel
        int nOutputChannels = input.getNChannels();
        if((input.getNChannels()==1) && (nWindows_>1))
        {
            LOUDNESS_DEBUG(name_ << ": Using parallel windows");
            parallelWindows_ = true;
            nOutputChannels = nWindows_;
            //if so, calculate the delay
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
        else
        {
            LOUDNESS_ASSERT(input.getNChannels() == nWindows_,
                    "Multiple channels but incorrect window specification.");
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
        output_.initialize(input.getNEars(), nOutputChannels, largestWindowSize_, input.getFs());
        output_.setFrameRate(input.getFrameRate());
        output_.setEffectiveSignalLengths(length_);

        return 1;
    }

    void Window::processInternal(const SignalBank &input)
    {
        Real* outputSignal;
        const Real* inputSignal;
        for(int ear=0; ear<input.getNEars(); ear++)
        {
            for(int w=0; w<nWindows_; w++)
            {
                inputSignal = input.getSignalReadPointer(ear, 0, windowOffset_[w]);
                outputSignal = output_.getSignalWritePointer(ear, w, 0);

                for(int smp=0; smp<length_[w]; smp++)
                    *outputSignal++ = window_[w][smp] * (*inputSignal++);
            }
        }
    }

    void Window::resetInternal()
    {
    }

    //Window functions:
    void Window::hann(RealVec &window, bool periodic)
    {
        unsigned int N = window.size();
        int denom = N-1;//produces zeros on both sides
        if(periodic)//Harris (1978) Eq 27b 
            denom = N;
        for(uint i=0; i<window.size(); i++)
        {
            window[i] = 0.5 - 0.5 * cos(2.0*PI*i/denom);
        }
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

    void Window::normaliseWindow(RealVec &window, const string &normalisation, double ref)
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
            LOUDNESS_WARNING(name_ << ": Normalisation must be 'energy' or 'amplitude', using 'energy'");
            normFactor = sqrt(wSize/sumSquares);
        }
        normFactor /= ref;
        for(uint i=0; i < wSize; i++)
            window[i] *= normFactor;
    }
 

    void Window::setRef(const Real ref)
    {
        ref_ = ref;
    }

}
