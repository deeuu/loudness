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

#include "../support/Module.h"

namespace loudness{

    class FixedRoexBank : public Module
    {

    public:
        FixedRoexBank(Real camLo=1.8,
                      Real camHi=38.9,
                      Real camStep=0.1,
                      Real level=51.0);

        virtual ~FixedRoexBank();

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();

        Real camLo_, camHi_, camStep_, level_;
        RealVecVec roex_;
    };
}

#endif
