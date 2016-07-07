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

#ifndef COMPLEXGAMMATONEBANK_H
#define COMPLEXGAMMATONEBANK_H

#include "../support/Module.h"
#include "../support/AuditoryTools.h"

namespace loudness{

    /** 
     * @class ComplexGammatoneBank
     *
     */
    class ComplexGammatoneBank : public Module
    {
    public:

        ComplexGammatoneBank(Real camLo = 2.0,
                         Real camHi = 40.0,
                         Real camStep = 1.0,
                         int order = 4,
                         bool outputEnvelope = true);

        virtual ~ComplexGammatoneBank();

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();

        Real camLo_, camHi_, camStep_;
        int order_;
        bool outputEnvelope_;
        RealVec realCoef_, imagCoef_, normFactor_;
        SignalBank realPrev_, imagPrev_;

    };
}

#endif
