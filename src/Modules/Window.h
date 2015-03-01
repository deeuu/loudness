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

#include "../Support/Module.h"

namespace loudness{

    /**
     * @class Window
     * 
     * @brief Simple Moving Average (Window) filter.
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
    class Window : public Module
    {
    public:

        /**
         * @brief Constructs an Window object.
         *
         * @param windowSize Number of samples in the sliding window.
         * @param average Set to true (default) to average the samples in the
         * window.
         * @param squareInput Set to true if the input samples should be squared
         * (default is false).
         */
        Window(const string &windowType, const IntVec& length, bool periodic, const string &normalisation, bool alignOutput);
        Window(const string &windowType, int length, bool periodic, const string &normalisation, bool alignOutput);
        Window();
        virtual ~Window();

        /**
         * @brief Normalises the window, typically for FFT usage.
         *
         * @param normalisation A string dictating the normalisation factor
         * applied to the window(s). 
         *
         * If "rms", the window is normalised such according to:
         * sqrt(mean(window^2)) = 1.0
         * This is useful for energy conservation when computing the power
         * spectrum of a windowed segment.
         *
         * If "amplitude", the window is normalised according to:
         * sum(window) = 1.0
         * This is useful for preserving the peak amplitude of an input sinusoid
         * when computing the magnitude spectrum of a windowed segment.
         *
         */
        void normaliseWindow(RealVec &window, const string &normalisation);
        void setNormalisation(const string &normalisation);

        //Window functions are available for naughty method swiping
        void generateWindow(RealVec &window, const string &windowType, bool periodic);

        void setAlignOutput(bool alignOutput);
        void setSum(bool sum);

        /**
         * @brief Set whether the input signal should be squared.
         */
        void setSquareInput(bool squareInput);
        void setSqrRoot(bool sqrRoot);

        bool getSum() const;
        bool getSquareInput() const;
        bool getSqrRoot() const;
        bool getAlignOutput() const;

    private:

        virtual bool initializeInternal(const SignalBank &input);
        virtual void processInternal(const SignalBank &input);
        virtual void resetInternal();

        //window functions
        void hann(RealVec &window, bool periodic);

        string windowType_;
        IntVec length_, windowOffset_;
        int nWindows_, largestWindowSize_;
        bool periodic_,average_, sum_, squareInput_;
        string normalisation_;
        bool sqrRoot_, alignOutput_;
        bool parallelWindows_;
        RealVecVec window_;
    };
}

#endif
