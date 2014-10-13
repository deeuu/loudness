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
     * than or equal to the frame size. In general, stick to powers of two.
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
        FrameGenerator(int frameSize = 1024, int hopSize = 512);
        virtual ~FrameGenerator();

        /**
         * @brief Sets the frame size in seconds.
         */
        void setFrameSize(int frameSize);

        /**
         * @brief Sets the hop size in seconds.
         */
        void setHopSize(int hopSize);

        /**
         * @brief Returns the total number of samples comprising the frame.
         */
        int getFrameSize() const;

        /**
         * @brief Returns the hop size in samples.
         */
        int getHopSize() const;

        int getAudioBufferSize() const;

    private:

        virtual bool initializeInternal(const SignalBank &input);
        virtual void processInternal(const SignalBank &input);
        virtual void resetInternal();

        int frameSize_, hopSize_, audioBufferSize_, inputBufferSize_;
        int count_, readIdx_, writeIdx_, initNFramesFull_, nFramesFull_;
        RealVec audioBuffer_;
    };
}

#endif
