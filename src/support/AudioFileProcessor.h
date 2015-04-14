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

#ifndef AUDIOFILEPROCESSOR_H
#define AUDIOFILEPROCESSOR_H

#include "Common.h"
#include "Model.h"
#include "Timer.h"
#include "../modules/AudioFileCutter.h"

namespace loudness{
    
    /**
     * @class AudioFileProcessor
     *
     * @brief A class for processing audio files with loudness models.
     *
     * This class is temporary and may be removed. It is hoped that this class
     * will be extended to save loudness features to disk.
     *
     * It was decided not to make this class inherit from Module because it has
     * no input or output SignalBank.
     *
     * At present, it can be used as a convenience class for processing audio
     * files with a specified loudness model to save time with input
     * configuration. It can also be used to measure the running time of a
     * loudness model.
     *
     * The input loudness model should NOT be initialised but be configured as
     * desired. This class will take care of object initialisation upon calling
     * initialize().
     *
     */
    class AudioFileProcessor
    {
    public:

        /**
         * @brief Constructs an AudioFileProcessor object.
         *
         * @param fileName  Path to the audio file.
         */

        AudioFileProcessor(const string& fileName);
        ~AudioFileProcessor();

        void initialize(Model& model);
        void process(Model& model);
        void processAllFrames(Model& model);
        void appendNFrames(int nFramesToAppend);
        /** Get the elapsed time consumed by the model when calling
         * processAllFrames() */
        Real getProcessingTime();

        /** Returns the duration of the audio file in seconds. */
        Real getDuration();

        void reset();
        
    private:

        string fileName_;
        int nFrames_, hopSize_;
        AudioFileCutter cutter_;
        Timer timer_;
    };
}
#endif
