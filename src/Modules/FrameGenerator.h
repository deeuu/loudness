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

#ifndef FRAMEGENERATOR_H
#define FRAMEGENERATOR_H

#include "../Support/Module.h"

namespace loudness{

    /**
     * @class FrameGenerator
     * 
     * @brief Generates a frame of samples from an input SignalBank.
     *
     * This algorithm was developed for use with PowerSpectrum. In the current
     * (not so flexible) implementation, the number of samples in the input
     * SignalBank cannot be greater than the hop size. If this happens, the hop
     * size is automatically adjusted. Furthermore, the hop size must be less
     * than or equal to the frame size, but does not have to be an integer
     * multiple of it's length.
     *
     * @todo Check the implementation is read/write safe.
     *
     * @author Dominic Ward
     */
    class FrameGenerator : public Module
    {
    public:

        /**
         * @brief Constructs an FrameGenerator object.
         *
         * @param frameSize Number of samples in the frame.
         * @param hopSize Number of samples to hop forward.
         */
        FrameGenerator(int frameSize = 1024, int hopSize = 512, bool startAtZero=false);
        virtual ~FrameGenerator();

        /**
         * @brief Sets the frame size in samples.
         */
        void setFrameSize(int frameSize);

        /**
         * @brief Sets the hop size in samples.
         */
        void setHopSize(int hopSize);

        /**
         * @brief Sets whether the write index starts at 0 or at the centre of
         * the frame (default).
         */
        void setStartAtZero(bool startAtZero);

        /**
         * @brief Returns the total number of samples comprising the frame.
         */
        int getFrameSize() const;

        /**
         * @brief Returns the hop size in samples.
         */
        int getHopSize() const;

        /**
         * @brief Returns the startAtZero state.
         */
        bool getStartAtZero() const;

        int getAudioBufferSize() const;

    private:

        virtual bool initializeInternal(const SignalBank &input);
        virtual void processInternal(const SignalBank &input);
        virtual void resetInternal();

        int frameSize_, hopSize_, audioBufferSize_, inputBufferSize_;
        int writeIdx_, overlap_, remainingSamples_;
        bool startAtZero_;
        SignalBank audioBufferBank_;
    };
}

#endif
