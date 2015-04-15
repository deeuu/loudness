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

#include "AudioFileProcessor.h"

namespace loudness{
    
    AudioFileProcessor::AudioFileProcessor(const string& fileName) :
        cutter_(fileName)
    {
        LOUDNESS_DEBUG("AudioFileProcessor: Constructed");
    }

    AudioFileProcessor::~AudioFileProcessor() {};

    void AudioFileProcessor::initialize(Model& model)
    {
        LOUDNESS_ASSERT(model.isDynamic(), "Model is not dynamic.");

        //initialise audio cutter
        cutter_.setFrameSizeInSeconds(1.0 / model.getRate());
        cutter_.initialize();

        timeStep_ = cutter_.getFrameSize() / (Real) cutter_.getFs();
        LOUDNESS_DEBUG("AudioFileProcessor: timestep : " << timeStep_);

        model.initialize(cutter_.getOutput());
        nFrames_ = cutter_.getNFrames();
    }

    void AudioFileProcessor::appendNFrames(int nFramesToAppend)
    {
        nFrames_ += nFramesToAppend;
    }

    void AudioFileProcessor::process(Model& model)
    {
        cutter_.process();
        model.process(cutter_.getOutput());
    }

    void AudioFileProcessor::processAllFrames(Model& model)
    {
        model.reset();
        timer_.tic();
        int frame = nFrames_;
        while(frame-- > 0)
        {
            cutter_.process();
            model.process(cutter_.getOutput());
        }
        timer_.toc();
        cutter_.reset();
    }

    Real AudioFileProcessor::getProcessingTime()
    {
        return timer_.getElapsedTime();
    }

    Real AudioFileProcessor::getDuration() const
    {
        return cutter_.getDuration();
    }

    Real AudioFileProcessor::getTimeStep() const
    {
        return timeStep_;
    }

    int AudioFileProcessor::getNFrames() const
    {
        return nFrames_;
    }

    void AudioFileProcessor::reset()
    {
        cutter_.reset();
    }
}
