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

#ifndef OctaveBank_H
#define OctaveBank_H

#include "../support/Module.h"

namespace loudness{

    /**
     * @class OctaveBank
     *
     * @brief Applies a bank of octave band pass filters to an input power spectrum.
     *
     * The response of the filters follow the maximally flat Butterworth
     * function given in ANSI S1.11:1986. The order of the Butterworth filter
     * should be specified when instantiating this class (default order is 3).
     * Third octave band filters can be used by constructing an instance of this
     * class with @ isThirdOctave set to true (the default). The output of each
     * filter can be transformed to decibels by setting @ isOutputInDecibels to
     * true (default is false).
     *
     * The default centre frequencies of the filters are:
     *      {25, 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315,
     *      400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000,
     *      6300, 8000, 10000, 12500, 16000, 20000};
     *
     * To change the number of filters and/or the centre frequencies, pass a
     * RealVec to setCentreFreqs() after instantiation.
     *
     * REFERENCES:
     *
     * ANSI. (1986). ANSI S1.11-1986: Specification for Octave-Band and
     * Fractional-Octave-Band Analog and Digital Filters.
     */
 
    class OctaveBank : public Module
    {

    public:

        OctaveBank (int order = 3, 
                    bool isThirdOctave = true, 
                    bool isOutputInDecibels = false);

        virtual ~OctaveBank();

        /** Sets the centre frequencies of the filters in Hz. @centreFreqs must
         * have at least 1 element. */
        void setCentreFreqs (RealVec centreFreqs);

    private:

        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();

        int order_;
        bool isThirdOctave_, isOutputInDecibels_;
        Real exponent_, qDesExponentiated_;
        RealVec centreFreqs_;
    };
}

#endif
