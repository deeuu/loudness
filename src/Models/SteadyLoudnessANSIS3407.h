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

#ifndef STEADYLOUDNESSANSIS3407_H
#define STEADYLOUDNESSANSIS3407_H

#include "../Support/Model.h"
#include <string>

namespace loudness{

    class SteadyLoudnessANSIS3407 : public Model
    {
        public:
            SteadyLoudnessANSIS3407();
            virtual ~SteadyLoudnessANSIS3407();

            void setDiotic(bool diotic);
            void setDiffuseField(bool diffuseField);
            void setFilterSpacing(Real filterSpacing);
            
        private:
            virtual bool initializeInternal(const SignalBank &input);

            Real filterSpacing_;
            bool diotic_, diffuseField_;
            int outerEarType_;
    }; 
}

#endif

