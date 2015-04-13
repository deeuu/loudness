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

#ifndef STEADYSTATELOUDNESSANSIS342007_H
#define STEADYSTATELOUDNESSANSIS342007_H

#include "../support/Model.h"
#include <string>

namespace loudness{

    /**
     * @brief Implementation of the ANSI S3.4 2007 steady-state loudness model.
     * 
     * This loudness model is for processing spectra only, i.e., there is no
     * time-frequency decomposition. The input SignalBank can have
     * multiple ears and channels but only one sample per channel which
     * specifies the intensity of a single component. Make sure that the centre
     * frequencies corresponding to each input component are set.
     *
     * It should be noted that this model makes use of polynomials to
     * approximate the variables involved in the specific loudness calculations,
     * rather than interpolation as used in the standard. See AuditoryTools to
     * see how the specific loudness parameters are computed.
     *
     * REFERENCES:
     *
     * ANSI. (2007). ANSI S3.4-2007. Procedure for the Computation of Loudness of
     * Steady Sounds.
     */
    class SteadyStateLoudnessANSIS342007 : public Model
    {
        public:
            SteadyStateLoudnessANSIS342007();
            virtual ~SteadyStateLoudnessANSIS342007();

            void setPresentationDiotic(bool isPresentationDiotic);

            void setResponseDiffuseField(bool isResponseDiffuseField);

            void setFilterSpacing(Real filterSpacing);
            
        private:
            virtual bool initializeInternal(const SignalBank &input);

            Real filterSpacing_;
            bool isPresentationDiotic_, isResponseDiffuseField_;
    }; 
}

#endif
