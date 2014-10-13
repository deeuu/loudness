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

    AudioFileCutter::AudioFileCutter(string fileName, int frameSize) :
        Module("AudioFileCutter"),
        fileName_(fileName),
        frameSize_(frameSize),
        fs_(0),
        sndFile_(nullptr)
    {}
    
    AudioFileCutter::~AudioFileCutter()
    {
        if(initialized_)
        {
            //delete [] audioBuffer_;
            if(sf_close(sndFile_)>0)
                LOUDNESS_ERROR("AudioFileCutter: Error closing audio file");
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
            LOUDNESS_ERROR("AudioFileCutter: Error closing Audio file.");
            return 0;
        }

        // Check we have a filename
        if(fileName_.empty())
        {
            LOUDNESS_ERROR("AudioFileCutter: No file name!");
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
                LOUDNESS_ERROR("AudioFileCutter: Failed to open audio file: " 
                        << fileName_ << ", " 
                        << sf_strerror(sndFile_));
                return 0;
            }
            else
            {
                LOUDNESS_DEBUG("AudioFileCutter: Audio file successfully loaded.");

                //total number of samples in the audio file and number of frames
                nSamples_ = fileInfo.frames;
                nFrames_ = ceil(nSamples_ / (float)frameSize_);
                duration_ = nSamples_/(Real)fileInfo.samplerate;
                fs_ = fileInfo.samplerate;

                //force audio buffer size to be a multiple of frameSize_
                if(nSamples_ < MAXAUDIOBUFFERSIZE)
                {
                    audioBufferSize_ = ceil(nSamples_ / (float)frameSize_) * frameSize_;
                }
                else
                {
                    audioBufferSize_ = MAXAUDIOBUFFERSIZE * fileInfo.channels;
                    audioBufferSize_ = ceil(audioBufferSize_ / (float)frameSize_) * frameSize_;
                }
                LOUDNESS_DEBUG("AudioFileCutter: Audio buffer size: " << audioBufferSize_);
                bufferIdx_ = 0;
                frame_ = 0;

                if(initialize)
                {
                    output_.initialize(fileInfo.channels, frameSize_,
                            fileInfo.samplerate);
                    audioBuffer_.resize(audioBufferSize_);
                }
                else
                {
                    if(fileInfo.samplerate!=output_.getFs())
                    {
                        LOUDNESS_WARNING("AudioFileCutter: Sampling frequency: " 
                                << fileInfo.samplerate << " but previous was: "
                                << output_.getFs() << ". Continuing anyway...");
                    }
                    if(fileInfo.channels!=output_.getNChannels())
                    {
                        LOUDNESS_WARNING("AudioFileCutter: Number of channels: " 
                                << fileInfo.channels << " but previous was: "
                                << output_.getFs() << ". Continuing anyway...");
                    }
                }
                
                return 1;
            }
        }
    }

    void AudioFileCutter::process()
    {
        //if initialised and we have an audio file
        if(initialized_ && sndFile_)
        {
            //If we have extrated all data from buffer, get more
            if((bufferIdx_ == 0) || (bufferIdx_ == audioBufferSize_))
            {
                int readCount = sf_readf_float(sndFile_, &audioBuffer_[0], audioBufferSize_);

                //Zero pad when towards the end
                if(readCount<audioBufferSize_)
                    fill(audioBuffer_.begin() + readCount, audioBuffer_.end(),0.0);

                LOUDNESS_DEBUG("AudioFileCutter: Number of samples extracted: " << readCount);

                frame_ = 0;
                bufferIdx_ = 0;
            }  

            LOUDNESS_DEBUG("AudioFileCutter: frame: " << frame_ 
                    << " buffer index: " << bufferIdx_);

            //clear the output signal bank
            output_.clear(); 

            //Fill the output signal bank
            int nSamples = output_.getNSamples();
            int nChannels = output_.getNChannels();
            for(int i=0; i<nSamples; i += nChannels)
            {
                for(int j=0; j<nChannels; j++)
                    output_.setSample(j, i, audioBuffer_[bufferIdx_ + i + j]);
            }         
            
            //increment the frame counter
            frame_ += 1;

            //update buffer index
            bufferIdx_ = frame_ * nSamples * nChannels;

            //push through processing pipeline if necessary
            if(targetModule_)
                targetModule_->process(output_);
        }
    }

    void AudioFileCutter::resetInternal()
    {
        loadAudioFile(false);
    }

    int AudioFileCutter::getNSamples() const
    {
        return nSamples_;
    }

    Real AudioFileCutter::getDuration() const
    {
        return duration_;
    }

    int AudioFileCutter::getFs() const
    {
        return fs_;
    }

    int AudioFileCutter::getNFrames() const
    {
        return nFrames_;
    }


}
