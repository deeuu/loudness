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

#ifndef  IIR_H
#define  IIR_H

#include "../support/Filter.h"

namespace loudness{

    /**
     * @class IIR
     *
     * @brief Performs IIR filtering of an input SignalBank using direct form 2.
     *
     * At present, this algorithm supports multiple ears but not multiple channels.
     *
     */
    class IIR : public Module, public Filter
    {
    public:
        /** Constructs an IIR with no filter coefficients */
        IIR();
        /** Constructs an IIR with a set of feedforward (bCoefs) and
         * feedback (aCoefs) coefficients */
        IIR(const RealVec &bCoefs, const RealVec &aCoefs);

        virtual ~IIR();
    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();

    };
}

#endif

