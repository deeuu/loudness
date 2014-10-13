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

    FrameGenerator::FrameGenerator(int frameSize, int hopSize) :
        Module("FrameGenerator"),
        frameSize_(frameSize),
        hopSize_(hopSize)
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
        inputBufferSize_ = input.getNSamples();
        
        //Hop size checks
        LOUDNESS_DEBUG(name_ << ": Input buffer size in samples: " << inputBufferSize_);
        if(inputBufferSize_ > hopSize_)
        {
            LOUDNESS_WARNING(name_ << 
                    ": Hop size cannot be greater than input buffer size" 
                    << "...automatically correcting.");
        }
        else if((hopSize_ % inputBufferSize_)>0)
        {
            LOUDNESS_WARNING(name_ << 
                    ": Hop size is not an integer multiple of the input buffer size" 
                    << "...automatically correcting.");
        }
        
        hopSize_ = inputBufferSize_*ceil(hopSize_/(Real) inputBufferSize_);
        LOUDNESS_DEBUG(name_ << ": Hop size in samples: " << hopSize_);

        /*  
        if((frameSize_ % hopSize_)>0)
        {
            LOUDNESS_WARNING(name_ << 
                    ": Frame size is not an integer multiple of the hop size" 
                    << "...automatically correcting.");

            frameSize_ = hopSize_ * ceil(frameSize_ / (Real)hopSize_);
        }
        */
        LOUDNESS_DEBUG(name_ << ": Frame size in samples: " << frameSize_);
    
        //The audio buffer must also be an integer multiple of inputBufSize_
        audioBufferSize_ = inputBufferSize_* ceil(frameSize_/(Real)inputBufferSize_);

        /*  
        if(hopSize_ > (audioBufferSize_ - inputBufferSize_))
            audioBufferSize_ += inputBufferSize_;
        */

        //OK, allocate memory
        audioBuffer_.assign(audioBufferSize_, 0.0);
        LOUDNESS_DEBUG(name_ << 
                ": Audio buffer size in samples: " 
                << audioBufferSize_);
        
        //Number of frames until we reach the end of buffer
        initNFramesFull_ = audioBufferSize_ / inputBufferSize_;
        nFramesFull_ = hopSize_ / inputBufferSize_; //will be an int >= 1
        LOUDNESS_DEBUG(name_
                << ": Number of process calls until we can extract first frame: " 
                << initNFramesFull_
                << "\n Number of process calls until we can extract further frames: " 
                << nFramesFull_);

        readIdx_ = 0;
        writeIdx_ = 0;
        count_ = 0;

        //initialise the output signal
        output_.initialize(1, frameSize_, input.getFs());
        output_.setFrameRate(input.getFs()/(Real)hopSize_);

        return 1;
    }

    void FrameGenerator::processInternal(const SignalBank &input)
    {

        //fill internal buffer
        for(int i=0; i<inputBufferSize_; i++)
            audioBuffer_[writeIdx_++] = input.getSample(0, i);
        //wrap write index
        writeIdx_ = writeIdx_ % audioBufferSize_;

        count_++;
        if(count_ == initNFramesFull_)
        {
            LOUDNESS_DEBUG(name_ << ": readIdx_ : " << readIdx_);

            //output frame
            for(int i=0; i<frameSize_; i++)
            {
                output_.setSample(0, i, audioBuffer_[readIdx_++]);
                readIdx_ = readIdx_ % audioBufferSize_;
            }
            readIdx_ = (writeIdx_ + hopSize_) % audioBufferSize_;

            output_.setTrig(1);

            count_ = 0;
            initNFramesFull_ = nFramesFull_;
        }
        else
            output_.setTrig(0);
    }

    void FrameGenerator::resetInternal()
    {
        initNFramesFull_ = audioBufferSize_ / inputBufferSize_;
        nFramesFull_ = hopSize_ / inputBufferSize_;
        readIdx_ = 0;
        writeIdx_ = 0;
        count_ = 0;
    }

    void FrameGenerator::setFrameSize(int frameSize)
    {
        frameSize_ = frameSize;
    }

    void FrameGenerator::setHopSize(int hopSize)
    {
        hopSize_ = hopSize;
    }

    int FrameGenerator::getFrameSize() const
    {
        return frameSize_;
    }

    int FrameGenerator::getHopSize() const
    {
        return hopSize_;
    }

    int FrameGenerator::getAudioBufferSize() const
    {
        return audioBufferSize_;
    }
}
