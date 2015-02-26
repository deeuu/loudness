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
        Window(const string &windowType, const IntVec& length, bool periodic);
        Window(const string &windowType, int length, bool periodic);
        Window();
        virtual ~Window();

        //Window functions are available for naughty method swiping
        void generateWindow(RealVec &window, const string &windowType, bool periodic);

        void setAlignOutput(bool alignOutput);
        void setSum(bool sum);

        /**
         * @brief Set whether the input signal should be squared.
         */
        void setSquareInput(bool squareInput);
        void setSqrRoot(bool sqrRoot);

        bool getSquareInput() const;

    private:

        virtual bool initializeInternal(const SignalBank &input);
        virtual void processInternal(const SignalBank &input);
        virtual void resetInternal();

        //window functions
        void hann(RealVec &window, bool periodic);

        string windowType_ = "hann";
        IntVec length_, windowOffset_;
        int nWindows_, largestWindowSize_;
        bool periodic_=true, average_=false, sum_=false, squareInput_=false;
        bool sqrRoot_=false, alignOutput_ = true;
        bool parallelWindows_;
        RealVecVec window_;

    };
}

#endif
