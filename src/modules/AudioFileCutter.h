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

#ifndef AUDIOFILECUTTER_H
#define AUDIOFILECUTTER_H

#include "../support/Module.h"
#include <sndfile.h>

//We can exceed this but only by +frameSize
#define MAX_BUFFER_SIZE 192000

namespace loudness{

    /**
     * @class AudioFileCutter
     * 
     * @brief Extracts a frame of samples from an audio file and puts the result
     * into an output SignalBank for processing.
     *
     * This class uses an internal buffer to bring a block of samples into
     * memory, from which frames are extracted.
     *
     * Calling \ref process will generate a single frame of samples. If the
     * frame size is less than the length of the audio file or the end of the
     * file is reached, the frame is padded with zeros.
     *
     * @todo Add MONOMIX case for downmixing stereo files.
     */
    class AudioFileCutter : public Module
    {
    public:
        /**
         * @brief Constructs an AudioFileCutter object.
         *
         * @param fileName  Path to the audio file.
         * @param frameSize Number of samples in the frame.
         */
        AudioFileCutter(const string& fileName, int frameSize = 512);

        virtual ~AudioFileCutter();

        /** Set the frameSize. */
        void setFrameSize(int frameSize);

        /** Set the frameSize in seconds. frameSize will be computed upon
         * initialisation. */
        void setFrameSizeInSeconds(Real frameSizeInSeconds);

        /** Sets the file name of the audio file to be loaded. */
        void setFileName(const string& fileName);

        /**
         * @brief Returns the duration of the audio file in seconds.
         */
        Real getDuration() const;

        /**
         * @brief Returns the total number of frames in the audio file.
         *
         * The final frame is padded with zeros if the length of the audio file
         * is not an integer multiple of the frame size.
         */
        int getNFrames() const;

    private:
        virtual bool initializeInternal(const SignalBank &input){return 0;};
        virtual bool initializeInternal();
        virtual void processInternal(const SignalBank &input){};
        virtual void processInternal();
        virtual void resetInternal();

        string fileName_;
        int frameSize_, nFrames_;
        int nSamplesToLoadPerChannel_, audioBufferSize_, bufferIdx_, frame_;
        SNDFILE* sndFile_;
        vector<float> audioBuffer_;
        Real duration_, frameSizeInSeconds_;

    };
}

#endif
