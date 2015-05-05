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

#ifndef PeakFollower_H
#define PeakFollower_H

#include "../support/Module.h"

namespace loudness{

    /**
     * @class PeakFollower
     *
     * @brief Implements a peak follower by outputting the absolute value of the input
     * sample if it is greater than the previous output sample. If this
     * evaluates to false, an exponential decay with time-constant defined by
     * releaseTime is applied to the peak. This essentially involves
     * subtracting a proportion of the previous output sample from itself.  
     * This module will filter all signals in the input SignalBank independently
     * and therefore gives an output SignalBank of the same shape.
     *
     * The time-step used in the calculation of filter coefficients is derived
     * from the input SignalBank's frame rate, which by default is equal to the
     * sampling frequency.
     */
    class PeakFollower : public Module
    {
    public:
 
        /** Constructs an PeakFollower an exponential decay defined by the
         * time-constant releaseTime in seconds */
        PeakFollower(Real releaseTime);

        virtual ~PeakFollower();

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();

        Real releaseTime_, releaseCoef_;
    };
}

#endif
