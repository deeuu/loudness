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
#ifndef  Biquad_H
#define  Biquad_H

#include "../Support/Filter.h"

namespace loudness{

    class Biquad : public Module, public Filter
    {
        public:

            Biquad();
            Biquad(const RealVec &bCoefs, const RealVec &aCoefs);

            void setCoefficientFs(const Real coefficientFs);
            virtual ~Biquad();

        private:
            virtual bool initializeInternal(const SignalBank &input);
            virtual void processInternal(const SignalBank &input);
            virtual void resetInternal();

            Real coefficientFs_ = 0;
    };
}
#endif
