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

#ifndef ROEXBANKANSIS342007_H
#define ROEXBANKANSIS342007_H

#include "../support/Module.h"

namespace loudness{

    /**
     * @class RoexBankANSIS342007
     *
     * @brief Applies a set of level dependent rounded exponential (roex)
     * filters to an input power spectrum. 
     *
     * This implementation follows:
     *
     * ANSI. (2007). ANSI S3.4-2007. Procedure for the Computation of Loudness
     * of Steady Sounds.
     */
    class RoexBankANSIS342007 : public Module
    {

    public:

        /** Constructs a roex filter bank with filters equally spaced
         * on the Cam scale.
         * 
         * @param camLo Frequency of the first roex filter in Cams.
         * @param camHi Frequency of the last roex filter in Cams.
         * @param camStep Interval between adjacent filters on the cam Scale.
         */
        RoexBankANSIS342007(Real camLo = 1.8, Real camHi = 38.9, Real camStep = 0.1);

        virtual ~RoexBankANSIS342007();

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();

        int nFilters_;
        Real camLo_, camHi_, camStep_;
        RealVec pu_, pl_, pcomp_, compLevel_;
    };
}

#endif
