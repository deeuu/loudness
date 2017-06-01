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
#ifndef  BIQUAD_H
#define  BIQUAD_H

#include "../support/Filter.h"

namespace loudness{

    /**
     * @class Biquad
     *
     * @brief Implements a biquad filter.
     *
     * @sa Filter
     */
    class Biquad : public Module, public Filter
    {
        public:

            /** Constructs a Biquad filter with no filter coefficients */
            Biquad();
            Biquad(const string &type_);
            /** Constructs a Biquad filter using the feedforward and feedback
             * coefficients bCoefs and aCoefs.
             * Both coefficient vectors should each be of size three.
             */
            Biquad(const RealVec &bCoefs, const RealVec &aCoefs);

            /** Given the sampling frequency from which the coefficients used to
             * construct (or given to) the Biquad were derived, this function
             * estimates a new set of coefficients for target sampling
             * frequency. The target sampling frequency refers to the one used
             * by the input SignalBank used to initialise the object.
             *
             * @param coefficientFs The sampling frequency of the original filter coefficients.
             */
            void setCoefficientFs(const Real coefficientFs);
            virtual ~Biquad();

        private:
            virtual bool initializeInternal(const SignalBank &input);
            virtual bool initializeInternal(){return 0;};
            virtual void processInternal(const SignalBank &input);
            virtual void processInternal(){};
            virtual void resetInternal();

            std::string type_;
            Real coefficientFs_ = 0;
    };
}
#endif
