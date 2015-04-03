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

#ifndef SMA_H
#define SMA_H

#include "../support/Module.h"

namespace loudness{

    /**
     * @class SMA
     * 
     * @brief Simple Moving Average (SMA) filter.
     *
     * This module outputs a simple moving average of the input samples using a
     * length @pwindowSize window. A running sum is achieved by setting
     * @paverage to false (the default is true). Furthermore, to square the
     * input (useful for a running mean square), set @psquareInput to true
     * (default is false). 
     *
     * The implementation is based on RunningSum unit generator in
     * SuperCollider.
     *
     * @author Dominic Ward
     */
    class SMA : public Module
    {
    public:

        /**
         * @brief Constructs an SMA object.
         *
         * @param windowSize Number of samples in the sliding window.
         * @param average Set to true (default) to average the samples in the
         * window.
         * @param squareInput Set to true if the input samples should be squared
         * (default is false).
         */
        SMA(int windowSize=5, bool average=true, bool squareInput=false);
        virtual ~SMA();

        /**
         * @brief Sets the window size in samples.
         */
        void setWindowSize(int windowSize);

        /**
         * @brief Set whether the summed samples should be averaged.
         */
        void setAverage(bool average);

        /**
         * @brief Set whether the input signal should be squared.
         */
        void setSquareInput(bool squareInput);

        int getWindowSize() const;
        bool getAverage() const;
        bool getSquareInput() const;

    private:

        virtual bool initializeInternal(const SignalBank &input);
        virtual void processInternal(const SignalBank &input);
        virtual void resetInternal();

        int windowSize_, bufferIdx_;
        bool average_, squareInput_;
        RealVec audioBuffer_, runningSumBuf_, safeRunningSumBuf_;
    };
}

#endif
