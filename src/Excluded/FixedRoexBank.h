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

#ifndef FIXEDROEXBANK_H
#define FIXEDROEXBANK_H

#include "../Support/Module.h"

/*
 * =====================================================================================
 *        Class:  FixedRoexBank
 *  Description:  Applies a set of level dependent rounded exponential (roex) filters
 *                to an input power spectrum to estimate the auditory representation
 *                (excitation pattern). This implementation follows a combination of:
 *
 *                Glasberg, B. R., & Moore, B. C. J. (1990). 
 *                Derivation of Auditory Filter Shapes From Notched-Noise Data.
 *                Hearing Research, 47, 103–138.
 *
 *                ANSI S3.4-2007. (2007). 
 *                Procedure for the Computation of Loudness of Steady Sounds.
 * =====================================================================================
 */

namespace loudness{

    class FixedRoexBank : public Module
    {

    public:
        /*
         *--------------------------------------------------------------------------------------
         *       Class:  FixedRoexBank
         *      Method:  FixedRoexBank :: RoexBankGM
         * Description:  Constructs a FixedRoexBank object.
         *  Parameters:    freqLo:  lowest filter center frequency of interest.
         *                 freqHi:  highest filter center frequency of interest.
         *               estep:  ERB filter step size (reciprocal of filter density).
         *--------------------------------------------------------------------------------------
         */ 
        FixedRoexBank(Real freqLo=50, Real freqHi=15e3, Real estep=0.25, Real level=51, bool spread=true);

        virtual ~FixedRoexBank();

    private:
        /*
         *--------------------------------------------------------------------------------------
         *       Class:  FixedRoexBank
         *      Method:  FixedRoexBank :: initializeInternal
         * Description:  Given the input SignalBank to process, designs the filter bank 
         *               and configures the output SignalBank.
         *               The output is an N channel x 1 sample SignalBank representing excitation 
         *               per filter.
         *  Parameters:  input:  The power spectrum to be filtered.
         *--------------------------------------------------------------------------------------
         */ 
        virtual bool initializeInternal(const SignalBank &input);
        /*
         *--------------------------------------------------------------------------------------
         *       Class:  FixedRoexBank
         *      Method:  FixedRoexBank :: ProcessInternal
         * Description:  Computes the level per ERB, roex filter shapes
         *               and performs cochlea filtering to estimate the excitation pattern on 
         *               an ERB scale.
         *  Parameters:  input:  The power spectrum to be filtered.
         *--------------------------------------------------------------------------------------
         */  
        virtual void processInternal(const SignalBank &input);

        virtual void resetInternal();

        int nFilters_;
        Real freqLo_, freqHi_, camStep_, level_;
        bool spread_;
        RealVecVec roex_;
    };
}

#endif
