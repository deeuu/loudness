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
        //The running sum is implemented according to the RunningSum unit
        //generator code as part of SuperCollider. In particular,
        //`safeRunningSum_' starts a new summation of window samples everytime
        //`the window is full'. At that point `runningSum_' takes over and
        //`safeRunningSum_' resets. This helps to reduce the accumulation of
        //rounding errors over the course of the objects lifetime.
        int bufSize = input.getNEars() * input.getNChannels();
        runningSumBuf_.assign(bufSize, 0);
        safeRunningSumBuf_.assign(bufSize, 0);
        audioBuffer_.assign(bufSize * windowSize_, 0);
        bufferIdx_ = 0;

        //initialise the output signal
        output_.initialize(input);
        return 1;
    }

    void SMA::processInternal(const SignalBank &input)
    {
        int bufferIdx = bufferIdx_;
        for (int ear = 0; ear < input.getNEars(); ear++)
        {
            for (int chn = 0; chn < input.getNChannels(); chn++)
            {
                //input output pointers
                const Real* inputSignal = input.getSignalReadPointer(ear, chn);
                Real* outputSignal = output_.getSignalWritePointer(ear, chn);

                //pointers to summers
                int earChnIdx = ear * input.getNChannels() + chn;
                Real* safeRunningSum = &safeRunningSumBuf_[earChnIdx];
                Real* runningSum = &runningSumBuf_[earChnIdx];

                //pointer to storage
                earChnIdx = ear * input.getNChannels() * windowSize_ + chn * windowSize_;
                Real* audioBuf = &audioBuffer_[earChnIdx + bufferIdx_];

                int i = 0;
                int nSamples = input.getNSamples();
                int remainingInputSamples = nSamples;
                bufferIdx = bufferIdx_;

                //could be done in one loop using mod indexing
                //but save a bunch of ifs per sample
                while (i < nSamples)
                {
                    //check if we have more input samples than needed for running sum
                    int loopSamples = min(windowSize_ - bufferIdx, remainingInputSamples);

                    //Compute running sum
                    double x = 0.0;
                    for (int j = 0; j < loopSamples; j++)
                    {
                        x = *inputSignal++;
                        if(squareInput_)
                            x *= x;

                        *safeRunningSum += x;
                        *runningSum += x - (*audioBuf); //y[n] = y[n-1] + x[n] - x[n-M]
                        *audioBuf++ = x;

                        if(average_)
                            *outputSignal++ = *runningSum / windowSize_;
                        else
                            *outputSignal++ = *runningSum;
                    }

                    i += loopSamples;
                    bufferIdx += loopSamples;

                    //wrap buffer index
                    if(bufferIdx == windowSize_)
                    {
                        bufferIdx = 0;
                        *runningSum = *safeRunningSum;
                        *safeRunningSum = 0.0;
                        audioBuf = &audioBuffer_[earChnIdx];
                    }

                }//while
            }//chn
        }//ear

       //update buffer idx for next call 
       bufferIdx_ = bufferIdx;
    }

    void SMA::resetInternal()
    {
        runningSumBuf_.assign(runningSumBuf_.size(), 0);
        safeRunningSumBuf_.assign(safeRunningSumBuf_.size(), 0);
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
