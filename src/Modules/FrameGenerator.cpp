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

#include "FrameGenerator.h"

namespace loudness{

    FrameGenerator::FrameGenerator(int frameSize, int hopSize, bool startAtZero) :
        Module("FrameGenerator"),
        frameSize_(frameSize),
        hopSize_(hopSize),
        startAtZero_(startAtZero)
    {}
    
    FrameGenerator::~FrameGenerator()
    {
    }

    bool FrameGenerator::initializeInternal(const SignalBank &input)
    {
        if(hopSize_ > frameSize_)
        {
            LOUDNESS_ERROR(name_ 
                    << ": Hop size cannot be greater than frame size");
            return 0;
        }

        //number of input samples per process call
        int inputBufferSize = input.getNSamples();
        
        //Hop size checks
        LOUDNESS_DEBUG(name_ << ": Input buffer size in samples: " << inputBufferSize);
        if(hopSize_ < inputBufferSize)
        {
            LOUDNESS_WARNING(name_ << 
                    ": Hop size cannot be less than input buffer size" 
                    << "...automatically correcting.");
            hopSize_ = inputBufferSize;
        }
        
        LOUDNESS_DEBUG(name_ << ": Hop size in samples: " << hopSize_);
        LOUDNESS_DEBUG(name_ << ": Frame size in samples: " << frameSize_);
    
        //a buffer for storing remaining input samples once frame is full
        audioBufferBank_.initialize(input.getNEars(), input.getNChannels(), inputBufferSize, input.getFs());
        
        if (startAtZero_)
            writeIdx_ = 0;
        else
            writeIdx_ = ceil((frameSize_-1)/2.0);

        remainingSamples_ = 0;
        overlap_ = frameSize_ - hopSize_;

        //initialise the output signal
        output_.initialize(input.getNEars(), input.getNChannels(), frameSize_, input.getFs());
        output_.setFrameRate(input.getFs()/(Real)hopSize_);
        output_.setTrig(false);

        return 1;
    }

    void FrameGenerator::processInternal(const SignalBank &input)
    {
        //pull all signals back by overlap samples
        if(writeIdx_ == frameSize_)
        {
            output_.pullBack(hopSize_);
            writeIdx_ = overlap_;
        }

        //copy to output (input buf can't be < than hopSize_ so safe)
        if(remainingSamples_)
        {
            output_.fillSignalBank(writeIdx_, audioBufferBank_, 0, remainingSamples_);
            writeIdx_ += remainingSamples_;
            remainingSamples_ = 0;
        }

        //Fill the output signal
        int nSamples = input.getNSamples();
        int nSamplesToFill = frameSize_ - writeIdx_;
        if (nSamplesToFill > nSamples)
            nSamplesToFill = nSamples;
        else
            remainingSamples_ = nSamples - nSamplesToFill;

        output_.fillSignalBank(writeIdx_, input, 0, nSamplesToFill);
        writeIdx_ += nSamplesToFill;

        //if samples remaining store them
        if(remainingSamples_)
        {
            int readIdx = nSamples - remainingSamples_;
            audioBufferBank_.fillSignalBank(0, input, readIdx, remainingSamples_);
        }

        //if frames worth -> output
        if(writeIdx_ == frameSize_)
            output_.setTrig(true);
        else
            output_.setTrig(false);
    }

    void FrameGenerator::resetInternal()
    {
        writeIdx_ = 0;
        remainingSamples_ = 0;
        audioBufferBank_.clear();
    }

    void FrameGenerator::setFrameSize(int frameSize)
    {
        frameSize_ = frameSize;
    }

    void FrameGenerator::setHopSize(int hopSize)
    {
        hopSize_ = hopSize;
    }

    void FrameGenerator::setStartAtZero(bool startAtZero)
    {
        startAtZero_ = startAtZero;
    }

    int FrameGenerator::getFrameSize() const
    {
        return frameSize_;
    }

    int FrameGenerator::getHopSize() const
    {
        return hopSize_;
    }

    bool FrameGenerator::getStartAtZero() const
    {
        return startAtZero_;
    }

}
