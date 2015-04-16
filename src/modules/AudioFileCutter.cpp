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

#include "AudioFileCutter.h"

namespace loudness{

    AudioFileCutter::AudioFileCutter(const string& fileName, int frameSize) :
        Module("AudioFileCutter"),
        fileName_(fileName),
        frameSize_(frameSize),
        frameSizeInSeconds_(0),
        duration_(0),
        fs_(0),
        gainInDecibels_(0),
        sndFile_(nullptr)
    {}
    
    AudioFileCutter::~AudioFileCutter()
    {
        if(sndFile_)
        {
            if(sf_close(sndFile_)>0)
                LOUDNESS_ERROR(name_ << ": Error closing audio file.");
        }
    }

    bool AudioFileCutter::initializeInternal()
    {
        // If the file is already opened, first close it
        if (sf_close(sndFile_) > 0)
        {
            LOUDNESS_ERROR(name_ << ": Error closing Audio file.");
            return 0;
        }

        // Check we have a valid filename
        if(fileName_.empty())
        {
            LOUDNESS_ERROR(name_ << ": No file name!");
            return 0;
        }

        // Open the audio file
        SF_INFO fileInfo;
        fileInfo.format = 0;
        sndFile_ = sf_open(fileName_.c_str(), SFM_READ, &fileInfo);

        if (!sndFile_) //if null
        {
            LOUDNESS_ERROR(name_ << ": Failed to open audio file: " 
                    << fileName_ << ", " 
                    << sf_strerror(sndFile_));
            return 0;
        }

        LOUDNESS_DEBUG(name_ << ": Audio file successfully loaded.");

        //total number of samples in the audio file and number of frames
        if (frameSizeInSeconds_ > 0)
            frameSize_ = int(round(fileInfo.samplerate * frameSizeInSeconds_));

        nFrames_ = ceil(fileInfo.frames / (float)frameSize_);
        duration_ = fileInfo.frames/(Real)fileInfo.samplerate;
        fs_ = fileInfo.samplerate;
 
        //force audio buffer size to be a multiple of frameSize_
        long long totalAudioSamples = fileInfo.frames * fileInfo.channels;
        if(totalAudioSamples < MAX_BUFFER_SIZE)
        {
            nSamplesToLoadPerChannel_ = ceil(fileInfo.frames 
                    / (float)frameSize_) * frameSize_;
        }
        else
        {
            nSamplesToLoadPerChannel_ = ceil((MAX_BUFFER_SIZE / (float)fileInfo.channels)
                    / (float)frameSize_) * frameSize_;
        } 

        std::cout << nSamplesToLoadPerChannel_ << std::endl;

        audioBufferSize_ = fileInfo.channels * nSamplesToLoadPerChannel_;

        LOUDNESS_DEBUG(name_ 
                << ": Sample rate: " << fileInfo.samplerate
                << "\n Number of samples per audio channel: " 
                << fileInfo.frames
                << "\n Number of audio channels: "
                << fileInfo.channels
                << "\n Audio buffer size: " << audioBufferSize_);

        bufferIdx_ =  audioBufferSize_;
        audioBuffer_.assign(audioBufferSize_, 0.0);

        linearGain_ = decibelsToAmplitude(gainInDecibels_);

        output_.initialize(fileInfo.channels, 1,
                frameSize_, fileInfo.samplerate);

        return 1;
    }

    void AudioFileCutter::processInternal()
    {
        //if we have an audio file
        if (sndFile_)
        {
            //If we have extrated all data from buffer, get more
            if (bufferIdx_ == audioBufferSize_)
            {
                int readCount = sf_readf_float(sndFile_, &audioBuffer_[0], nSamplesToLoadPerChannel_);

                LOUDNESS_PROCESS_DEBUG(name_ 
                        << ": Number of samples extracted per audio channel: "
                        << readCount);

                //Zero pad when towards the end
                int remainingSamplesPerChannel = nSamplesToLoadPerChannel_ - readCount;
                if (remainingSamplesPerChannel > 0)
                {
                    fill(audioBuffer_.begin() + readCount * output_.getNEars(), audioBuffer_.end(),0.0);

                    LOUDNESS_PROCESS_DEBUG(name_ << ": Padding " 
                            << remainingSamplesPerChannel
                            << " sample with zeros per audio channel");
                }

                bufferIdx_ = 0;
            }  

            //clear the output signal bank
            output_.zeroSignals();

            //Fill the output signal bank
            int nEars = output_.getNEars();
            for (int ear = 0; ear < nEars; ear ++)
            {
                float* inputSignal = &audioBuffer_[bufferIdx_ + ear];
                Real* outputSignal = output_.getSignalWritePointer(ear, 0, 0);
                int smp = output_.getNSamples();
                while(smp-- > 0)
                {
                    *outputSignal++ = *inputSignal;
                    inputSignal += nEars;
                }
            }

            //apply gain
            output_.scale(linearGain_);
            
            //update buffer index
            bufferIdx_ += output_.getNSamples() * nEars;
        }
    }

    void AudioFileCutter::setGainInDecibels(Real gainInDecibels)
    {
        gainInDecibels_ = gainInDecibels;
    }

    void AudioFileCutter::setFrameSize(int frameSize)
    {
        frameSize_ = frameSize;
    }

    void AudioFileCutter::setFrameSizeInSeconds(Real frameSizeInSeconds)
    {
        frameSizeInSeconds_ = frameSizeInSeconds;
    }

    void AudioFileCutter::setFileName(const string& fileName)
    {
        fileName_ = fileName;
    }

    void AudioFileCutter::resetInternal()
    {
        if(sndFile_)
        {
            audioBuffer_.assign(audioBuffer_.size(), 0.0);
            bufferIdx_ =  audioBufferSize_;
            sf_seek(sndFile_, 0, SEEK_SET);
        }
    }

    int AudioFileCutter::getFrameSize() const
    {
        return frameSize_;
    }

    int AudioFileCutter::getFs() const
    {
        return fs_;
    }

    Real AudioFileCutter::getDuration() const
    {
        return duration_;
    }

    int AudioFileCutter::getNFrames() const
    {
        return nFrames_;
    }
}
