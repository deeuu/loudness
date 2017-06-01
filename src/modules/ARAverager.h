/*
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

#ifndef ARAVERAGER_H
#define ARAVERAGER_H

#include "../support/Module.h"

namespace loudness{

    /**
     * @class ARAverager
     *
     * @brief Implements a first-order low-pass filter with seperate attack and
     * release time-constants. This module will filter all signals in the input
     * SignalBank independently and therefore gives an output SignalBank of the
     * same shape.
     *
     * The time-step used in the calculation of filter coefficients is derived
     * from the input SignalBank's frame rate, which by default is equal to the
     * sampling frequency.
     *
     * REFERENCES:
     *
     * Zolzer, U. (2002). DAFX - Digital Audio Effects. (U. Zolzer, Ed.) 
     * John Wiley & Sons, Ltd.
     *
     * Glasberg, B. R., & Moore, B. C. J. (2002). A Model of Loudness Applicable
     * to Time-Varying Sounds. Journal of the Audio Engineering Society, 50(5),
     * 331–342.
     *
     */
    class ARAverager : public Module
    {
    public:
 
        /** Constructs an ARAverager with attackTime and releaseTime in
         * seconds */
        ARAverager(Real attackTime=0.1, Real releaseTime=0.1);

        virtual ~ARAverager();

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();

        Real attackTime_, releaseTime_;
        Real attackCoef_, releaseCoef_;
    };
}

#endif
