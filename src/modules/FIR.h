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

#ifndef FIR_H
#define FIR_H

#include "../support/Filter.h"


namespace loudness{

    /** 
     * @class FIR
     *
     * @brief Performs FIR filtering of an input SignalBank using direct form 2.
     *
     * At present, this algorithm supports multiple ears but not multiple channels.
     *
     * @sa Filter
     */
    class FIR : public Module, public Filter
    {
    public:

        /** Constructs an FIR with no filter coefficients */
        FIR();
        /** Constructs an FIR with a set of feedforward coefficients */
        FIR(const RealVec &bCoefs);

        virtual ~FIR();

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();
    };
}

#endif
