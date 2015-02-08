/*
 * Copyright (C) 2014 Dominic Ward <contactdominicward@CHail.com>
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

#ifndef DYNAMICLOUDNESSCH_H
#define DYNAMICLOUDNESSCH_H

#include "../Support/Model.h"

namespace loudness{

    /**
     * @class DynamicLoudnessCH
     *
     * @brief Implements Chen and Hu's time-varying loudness model.
     *
     * At present, there are two parameter sets available:
     *
     * 1. CH12 - The specification used by Chen and Hu (2012).
     * 2. FASTER1 - Uses a compressed spectrum with a filter spacing of 0.25
     * Cams.
     *
     * The default is FASTER1.
     *
     * Use loadParameterSet() to select the model parameters.
     * 
     * If you want to use a time-domain filter for simulating the transmission
     * response of the outer and middle ear, such as the 4096 order FIR filter
     * used by Glasberg and Moore, then specify the path (string) to the filter
     * coefficients when constructing the object. The coefficients must be a
     * Numpy array stored as a binary file in the '.npy' format. If a file path
     * is not provided, pre-cochlear filtering is performed using the frequency-domain
     * weighting function as specified by Chen and Hu (2012). The middle ear
     * function follows the one given in Figure 2 of Chen et al (2011).
     *
     * REFERENCES:
     *
     * Chen, Z., Hu, G., Glasberg, B. R., & Moore, B. C. J. (2011). A new method
     * of calculating auditory excitation patterns and loudness for steady
     * sounds. Hearing Research, 282(1-2), 204–15.
     *
     * Chen, Z., & Hu, G. (2012). A revised method of calculating auditory
     * excitation patterns and loudness for time-varying sounds. In Proceedings
     * of the IEEE International Conference on Acoustics, Speech, and Signal
     * Processing (ICASSP ’12) (pp. 157–160).
     *
     */
    class DynamicLoudnessCH : public Model
    {
        public:

            enum ParameterSet{
                CH02 = 0 /**< Chen and Hu 2002. */,
                FASTER1 = 1 /**< Compressed spectrum and fast roex filterbank. */
            };

            /**
             * @brief Constructs a model with a path to the '.npy' file holding
             * the pre-cochlear filter coefficients.
             *
             * If no path is given, the hybrid filter will
             * perform the outer and middle ear filtering.
             */
            DynamicLoudnessCH(string pathToFilterCoefs = "");

            virtual ~DynamicLoudnessCH();

            /**
             * @brief Loads a parameter set.
             */
            void loadParameterSet(ParameterSet set);
            void setDiffuseField(bool diffuseField);
            void setGoertzel(bool goertzel);
            void setDiotic(bool diotic);
            void setUniform(bool uniform);
            void setFilterSpacing(Real filterSpacing);
            void setCompressionCriterion(Real compressionCriterion);

        private:
            virtual bool initializeInternal(const SignalBank &input);

            string pathToFilterCoefs_;
            int outerEarType_;
            Real filterSpacing_, compressionCriterion_;
            bool uniform_, diotic_, goertzel_, diffuseField_;
    }; 
}

#endif
