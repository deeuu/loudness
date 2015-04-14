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

#include "../support/Module.h"

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
     * FrameGenerator can generate frames for signals in the inputSignalBank,
     * and so multiple ear/channel processing is supported.
     */
    class FrameGenerator : public Module
    {
    public:

        /**
         * @brief Constructs an FrameGenerator object.
         *
         * @param frameSize Number of samples in the frame.
         * @param hopSize Number of samples to hop forward.
         * @param startAtCentreOfFrame Set true to align the first input sample
         * with the centre of the frame. If false, the frame is filled starting
         * at sample index zero.
         */
        FrameGenerator(int frameSize = 1024, int hopSize = 512, bool startAtFrameCentre = false);
        virtual ~FrameGenerator();

        /**
         * @brief Returns the total number of samples comprising the frame.
         */
        int getFrameSize() const;

        /**
         * @brief Returns the hop size in samples.
         */
        int getHopSize() const;

        /**
         * @brief Returns true if the data begins at the centre of the analysis
         * frame.
         */
        bool getstartAtCentreOfFrame() const;

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();

        int frameSize_, hopSize_, audioBufferSize_, inputBufferSize_;
        int writeIdx_, overlap_, remainingSamples_;
        bool startAtCentreOfFrame_;
        SignalBank audioBufferBank_;
    };
}

#endif
