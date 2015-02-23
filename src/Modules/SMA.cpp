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

#include "SMA.h"

namespace loudness{

    SMA::SMA(int windowSize, bool average, bool squareInput) :
        Module("SMA"),
        windowSize_(windowSize),
        average_(average),
        squareInput_(squareInput)
    {}
    
    SMA::~SMA()
    {
    }

    bool SMA::initializeInternal(const SignalBank &input)
    {
        audioBufferSize_ = windowSize_;
        audioBuffer_.assign(windowSize_, 0.0);
        bufferIdx_ = 0;
        //The running sum is implemented according to the RunningSum unit
        //generator code as part of SuperCollider. In particular,
        //`safeRunningSum_' starts a new summation of window samples everytime
        //`the window is full'. At that point `runningSum_' takes over and
        //`safeRunningSum_' resets. This helps to reduce the accumulation of
        //rounding errors over the course of the objects lifetime.
        runningSum_ = 0.0;
        safeRunningSum_ = 0.0;

        //initialise the output signal
        output_.initialize(input);
        return 1;
    }

    void SMA::processInternal(const SignalBank &input)
    {

        double x = 0.0;
        int nSamples = input.getNSamples();
        int remainingInputSamples = nSamples;
        int i=0;
        while(i<nSamples)
        {
            //check if we have more input samples than needed for running sum
            int loopSamples = audioBufferSize_ - bufferIdx_;
            remainingInputSamples = nSamples - i;
            if(loopSamples > remainingInputSamples)
                loopSamples = remainingInputSamples;

            //Compute running sum
            for(int j=0; j<loopSamples; j++)
            {
                x = input.getSample(0, i);
                if(squareInput_)
                    x *= x;
                safeRunningSum_ += x;
                runningSum_ = runningSum_ + x - audioBuffer_[bufferIdx_];
                audioBuffer_[bufferIdx_++] = x;

                //output every sample
                if(average_)
                    output_.setSample(0, i, runningSum_ / windowSize_);
                else
                    output_.setSample(0, i, runningSum_);
                i++;
            }

            //wrap buffer index
            if(bufferIdx_ == audioBufferSize_)
            {
                bufferIdx_ = 0;
                runningSum_ = safeRunningSum_;
                safeRunningSum_ = 0.0;
            }
        }
    }

    void SMA::resetInternal()
    {
        runningSum_ = 0.0;
        safeRunningSum_ = 0.0;
        bufferIdx_ = 0;
    }
 

    void SMA::setWindowSize(int windowSize)
    {
        windowSize_ = windowSize;
    }

    void SMA::setAverage(bool average)
    {
        average_ = average;
    }

    void SMA::setSquareInput(bool squareInput)
    {
        squareInput_ = squareInput;
    }

    int SMA::getWindowSize() const
    {
        return windowSize_;
    }

    bool SMA::getAverage() const
    {
        return average_;
    }

    bool SMA::getSquareInput() const
    {
        return squareInput_;
    }
}
