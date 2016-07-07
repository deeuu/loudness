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
        runningSumBuf_.initialize(
                       input.getNSources(),
                       input.getNEars(),
                       input.getNChannels(),
                       2, // one sample for safe sum and another for main sum
                       input.getFs());

        audioBuffer_.initialize(
                     input.getNSources(),
                     input.getNEars(),
                     input.getNChannels(),
                     windowSize_,
                     input.getFs());

        bufferIdx_ = 0;

        //initialise the output signal
        output_.initialize(input);
        return 1;
    }

    void SMA::processInternal(const SignalBank &input)
    {
        int bufferIdx = bufferIdx_;
        for (int src = 0; src < input.getNSources(); ++src)
        {
            for (int ear = 0; ear < input.getNEars(); ++ear)
            {
                for (int chn = 0; chn < input.getNChannels(); ++chn)
                {
                    //input output pointers
                    const Real* inputSignal = input.getSignalReadPointer
                                              (src, ear, chn);
                    Real* outputSignal = output_.getSignalWritePointer
                                         (src, ear, chn);

                    Real* runningSum = runningSumBuf_
                                       .getSignalWritePointer
                                       (src, ear, chn);

                    //pointer to storage
                    Real* audioBuf = audioBuffer_.getSignalWritePointer
                                     (src, ear, chn, bufferIdx_);

                    int i = 0;
                    int nSamples = input.getNSamples();
                    int remainingInputSamples = nSamples;
                    bufferIdx = bufferIdx_;

                    //could be done in one loop using mod indexing
                    //but save a bunch of ifs per sample
                    while (i < nSamples)
                    {
                        //check if we have more input samples than needed for running sum
                        int loopSamples = min(windowSize_ - bufferIdx,
                                              remainingInputSamples);

                        //Compute running sum
                        for (int smp = 0; smp < loopSamples; ++smp)
                        {
                            Real x = inputSignal[smp];
                            if(squareInput_)
                                x *= x;

                            runningSum[0] += x; //safe
                            runningSum[1] += x - audioBuf[smp]; //y[n] = y[n-1] + x[n] - x[n-M]
                            audioBuf[smp] = x;

                            if(average_)
                                outputSignal[smp] = runningSum[1] / windowSize_;
                            else
                                outputSignal[smp] = runningSum[1];
                        }

                        i += loopSamples;
                        bufferIdx += loopSamples;

                        //wrap buffer index
                        if(bufferIdx == windowSize_)
                        {
                            bufferIdx = 0;
                            runningSum[1] = runningSum[0];
                            runningSum[0] = 0.0;
                            audioBuf = audioBuffer_.getSignalWritePointer
                                       (src, ear, chn);

                        }
                    }//while
                }//chn
            }//ear
        }//src

       //update buffer idx for next call 
       bufferIdx_ = bufferIdx;
    }

    void SMA::resetInternal()
    {
        runningSumBuf_.zeroSignals();
        audioBuffer_.zeroSignals();
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
