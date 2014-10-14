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

#ifndef DYNAMICLOUDNESSGM_H
#define DYNAMICLOUDNESSGM_H

#include "../Support/Model.h"

namespace loudness{

    /**
     * @class DynamicLoudnessGM
     *
     * @brief Implements Glasberg and Moore's time-varying loudness model.
     *
     * At present, there are two parameter sets available:
     *
     * 1. GM02 - The specification used by Glasberg and Moore (2002).
     * 2. FASTER1 - Uses a compressed spectrum with hybrid outer and middle ear
     * filter.
     *
     * If you want to use a time-domain filter for the outer and middle ear,
     * such as the 4096 order free-field FIR filter used by Glasberg and Moore,
     * then use call setPathToFilterCoefs(). The coefficients must be a '.npy'
     * file. If no coefficients are provided, the hybrid filter scheme will be
     * used.
     */
    class DynamicLoudnessGM : public Model
    {
        public:

            enum ParameterSet{
                GM02 = 0 /**< Glasberg and Moore 2002. */,
                FASTER1 = 1 /**< Compressed spectrum with HPF and spectral weighting. */
            };

            /**
             * @brief Constructs a model with a path to the outer and middle ear
             * filter coefficients.
             *
             * If no path is given (empty string), then the hybrid filter will
             * peform the outer and middle ear filtering.
             *
             * The default parameter set is FASTER1.
             */
            DynamicLoudnessGM(string pathToFilterCoefs = "");

            virtual ~DynamicLoudnessGM();

            void loadParameterSet(ParameterSet set);

            void setTimeStep(Real timeStep);
            void setHpf(bool hpf);
            void setDiffuseField(bool diffuseField);
            void setGoertzel(bool goertzel);
            void setDiotic(bool diotic);
            void setUniform(bool uniform);
            void setInterpRoexBank(bool interpRoexBank);
            void setFilterSpacing(Real filterSpacing);
            void setCompressionCriterion(Real compressionCriterion);
            void setAnsiBank(bool ansiBank);
            void setPathToFilterCoefs(string pathToFilterCoefs);
            void setFastBank(bool fastBank);

            Real getTimeStep() const;
            
        private:
            virtual bool initializeInternal(const SignalBank &input);

            int outerEarType_;
            Real timeStep_, filterSpacing_, compressionCriterion_;
            bool ansiBank_, fastBank_, interpRoexBank_, uniform_, diotic_, goertzel_;
            bool hpf_, diffuseField_;
            string pathToFilterCoefs_;

    }; 
}

#endif

