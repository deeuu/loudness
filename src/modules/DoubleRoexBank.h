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

#ifndef DOUBLEROEXBANK_H
#define DOUBLEROEXBANK_H

#include "../support/Module.h"
#include "../thirdParty/spline/Spline.h"

namespace loudness{

    /**
     * @class DoubleRoexBank
     *
     * @brief Applies a bank of double roex filters to an input power spectrum.
     *
     * This is an implementation of the double roex filter model proposed by
     * Chen et al (2011). The model makes use of a passive wideband filter and
     * an active narrowband filter.  The output of the passive filter controls
     * the active gain of the narrowband filter.
     *
     * REFERENCES:
     *
     * Chen, Z., Hu, G., Glasberg, B. R., & Moore, B. C. J. (2011). A new method
     * of calculating auditory excitation patterns and loudness for steady
     * sounds.  Hearing Research, 282(1-2), 204–15.
     *
     */
 
    class DoubleRoexBank : public Module
    {

    public:

        /** Constructs a DoubleRoexBank with double roex filters equally spaced
         * on the Cam scale.
         * 
         * @param camLo Frequency of the first double roex filter in Cams.
         * @param camHi Frequency of the last double roex filter in Cams.
         * @param camStep Interval between adjacent filters on the cam Scale.
         * @param scalingFactor A scalar applied to the output excitation
         * pattern (default is 1).
         * @param isExcitationPatternInterpolated Set true to interpolate the
         * excitation pattern.  
         * @param isInterpolationCubic Set true for cubic
         * interpolation, false for linear. Only applies if
         * isExcitationPatternInterpolated is true;
         
         */
        DoubleRoexBank(Real camLo = 1.5,
                Real camHi = 40.2,
                Real camStep = 0.1,
                Real scalingFactor = 1.0,
                bool isExcitationPatternInterpolated = false,
                bool isInterpolationCubic = true);

        virtual ~DoubleRoexBank();

    private:

        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();

        Real camLo_, camHi_, camStep_, scalingFactor_;
        bool isExcitationPatternInterpolated_, isInterpolationCubic_;
        int nFilters_;
        RealVec maxGdB_, thirdGainTerm_, cams_, logExcitation_;
        RealVecVec wPassive_, wActive_;
        spline spline_;
    };
}

#endif
