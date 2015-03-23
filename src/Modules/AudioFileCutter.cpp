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
        sndFile_(nullptr)
    {}
    
    AudioFileCutter::~AudioFileCutter()
    {
        if(initialized_)
        {
            //delete [] audioBuffer_;
            if(sf_close(sndFile_)>0)
                LOUDNESS_ERROR("AudioFileCutter: Error closing audio file.");
        }
    }

    bool AudioFileCutter::initialize()
    {
        if(loadAudioFile(true))
        {
            LOUDNESS_DEBUG("Module: Module initialized.");
            if(output_.isInitialized())
            {
                if(targetModule_)
                    targetModule_ -> initialize(output_);
            }
            initialized_ = 1;
            return 1;
        }
        else
        {
            LOUDNESS_ERROR("Module: Module not initialized!");
            return 0;
        }
    }

    bool AudioFileCutter::loadAudioFile(bool initialize)
    { 
        // If the file is already opened, first close it
        if (sf_close(sndFile_)>0)
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
        else
        {
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
            else
            {
                LOUDNESS_DEBUG(name_ << ": Audio file successfully loaded.");

                //total number of samples in the audio file and number of frames
                nFrames_ = ceil(fileInfo.frames / (float)frameSize_);
                duration_ = fileInfo.frames/(Real)fileInfo.samplerate;

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

                audioBufferSize_ = fileInfo.channels * nSamplesToLoadPerChannel_;
                bufferIdx_ =  audioBufferSize_;
                LOUDNESS_DEBUG(name_ << "Sample rate: " << fileInfo.samplerate
                        << "\n Number of samples per audio channel: " 
                        << totalAudioSamples/fileInfo.channels
                        << "\n Number of audio channels: "
                        << fileInfo.channels
                        << "\n Audio buffer size: " << audioBufferSize_);

                if(initialize)
                {
                    output_.initialize(fileInfo.channels, 1,
                            frameSize_, fileInfo.samplerate);
                    audioBuffer_.resize(audioBufferSize_);
                }
                else
                {
                    if(fileInfo.samplerate != output_.getFs())
                    {
                        LOUDNESS_WARNING(name_ << ": Sampling frequency: " 
                                << fileInfo.samplerate << " but previous was: "
                                << output_.getFs() << ". Continuing anyway...");
                    }
                    if(fileInfo.channels != output_.getNChannels())
                    {
                        LOUDNESS_WARNING(name_ << ": Number of channels: " 
                                << fileInfo.channels << " but previous was: "
                                << output_.getNChannels() << ". Continuing anyway...");
                    }
                }
                
                return 1;
            }
        }
    }

    void AudioFileCutter::process()
    {
        //if initialised and we have an audio file
        if (initialized_ && sndFile_)
        {
            //If we have extrated all data from buffer, get more
            if (bufferIdx_ == audioBufferSize_)
            {
                int readCount = sf_readf_float(sndFile_, &audioBuffer_[0], nSamplesToLoadPerChannel_);

                LOUDNESS_DEBUG(name_ <<
                        ": Number of samples extracted per audio channel: "
                        << readCount);

                //Zero pad when towards the end
                int remainingSamplesPerChannel = nSamplesToLoadPerChannel_ - readCount;
                if (remainingSamplesPerChannel > 0)
                {
                    fill(audioBuffer_.begin() + readCount * output_.getNEars(), audioBuffer_.end(),0.0);
                    LOUDNESS_DEBUG(name_ << ": Padding " 
                            << remainingSamplesPerChannel
                            << " sample with zeros per audio channel");
                }

                bufferIdx_ = 0;
            }  

            //clear the output signal bank
            output_.clear(); 

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
            
            //update buffer index
            bufferIdx_ += output_.getNSamples() * nEars;

            //push through processing pipeline if necessary
            if(targetModule_)
                targetModule_->process(output_);
        }
    }

    void AudioFileCutter::resetInternal()
    {
        loadAudioFile(false);
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
