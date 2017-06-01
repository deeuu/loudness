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

#ifndef FORWARDMASKINGPO1998_H
#define FORWARDMASKINGPO1998_H

#include "../support/Filter.h"

namespace loudness{

    class ForwardMaskingPO1998 : public Module, public Filter
    {
        public:
            ForwardMaskingPO1998(
                    Real timeConstant1=4e-3,
                    Real timeConstant2=29e-3,
                    Real weight=0.0251);

            virtual ~ForwardMaskingPO1998();

        private:
            virtual bool initializeInternal(const SignalBank &input);
            virtual bool initializeInternal(){return 0;};
            virtual void processInternal(const SignalBank &input);
            virtual void processInternal(){};
            virtual void resetInternal();

            Real timeConstant1_, timeConstant2_, weight_;
    };
}

#endif
