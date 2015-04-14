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

#ifndef Window_H
#define Window_H

#include "../support/Module.h"

namespace loudness{

    /**
     * @class Window
     * 
     * @brief Applies a window function to the input SignalBank.
     *
     * This module is for use with PowerSpectrum. In the current implementation,
     * multiple ears are supported but each ear is expected to have one channel
     * (one signal per ear to be windowed). If multiple window sizes are
     * specified when constructed this module, multiple windowed frames of
     * varying sizes can be generated from a single signal. In this case, the
     * analysis frames are output in separate channels for each ear. If the
     * number of input samples is greater than the window size(s), the analysis
     * windows are zero-padded after the end of the window.
     *
     * Currently supported windows:
     * - Hann (periodic or symmetric)
     */
    class Window : public Module
    {
    public:

        /** Constructs a Window object for generating a single frame.
         *
         * @param windowType The type of window to apply.
         * @param length The length of the window in samples.
         * @param periodic Periodic if true, symmetric otherwise.
         */
        Window(const string &windowType, int length, bool periodic);

        /** Constructs a Window object for generating a multiple frames.
         *
         * @param windowType The type of window to apply.
         * @param length A vector of window lengths in samples.
         * @param periodic Periodic if true, symmetric otherwise.
         */
        Window(const string &windowType, const IntVec& length, bool periodic);
        Window();
        virtual ~Window();

        /**
         * @brief Normalises the window, typically for FFT usage.
         *
         * @param normalisation A string dictating the normalisation factor
         * applied to the window(s). 
         *
         * If "energy", the window is normalised such according to:
         * mean(window^2) = 1.0
         * This is useful for energy conservation when computing the power
         * spectrum of a windowed segment.
         *
         * If "amplitude", the window is normalised according to:
         * mean(window) = 1.0
         * This is useful for preserving the peak amplitude of an input sinusoid
         * when computing the magnitude spectrum of a windowed segment.
         *
         */
        void normaliseWindow(RealVec &window, const string &normalisation, Real ref);
        void setNormalisation(const string &normalisation);

        //Window functions are available for naughty method swiping
        void generateWindow(RealVec &window, const string &windowType, bool periodic);

        void setRef(const Real ref);

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();

        //window functions
        void hann(RealVec &window, bool periodic);

        string windowType_;
        IntVec length_;
        bool periodic_, alignOutput_;
        string normalisation_;
        Real ref_;
        int nWindows_, largestWindowSize_;
        bool parallelWindows_;
        RealVecVec window_;
        IntVec windowOffset_;
    };
}

#endif
