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

#ifndef DYNAMICLOUDNESSCH2012_H
#define DYNAMICLOUDNESSCH2012_H

#include "../support/Model.h"

namespace loudness{

    /**
     * @class DynamicLoudnessCH2012
     *
     * @brief Implements Chen and Hu's (2012) time-varying loudness model.
     *
     * At present, there are two parameter sets available:
     *
     * 1. CH2012 - The specification used by Chen and Hu (2012).
     * 2. faster - Uses a compressed spectrum with a filter spacing of 0.25
     * Cams.
     *
     * The default is `faster'.
     *
     * Use configureModelParameters() to select the model parameters.
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
    class DynamicLoudnessCH2012 : public Model
    {
        public:

            /** Constructs a model with a path to the '.npy' file holding
             * the pre-cochlear filter coefficients.
             */
            DynamicLoudnessCH2012(const string& pathToFilterCoefs);
            /** Constructs a model with no time-domain filtering.
             * The pre-cochlear filter is performed using a frequency-domain
             * weighting function.
             */
            DynamicLoudnessCH2012();

            virtual ~DynamicLoudnessCH2012();

            void configureModelParameters(const string& setName);

            void setSampleSpectrumUniformly(bool sampleSpectrumUniformly);
            void setDioticPresentation(bool dioticPresentation);
            void setUseDiffuseFieldResponse(bool useDiffuseFieldResponse);
            void setStartAtWindowCentre(bool startAtWindowCentre);
            void setFilterSpacing(Real filterSpacing);
            void setCompressionCriterion(Real compressionCriterion);
            void setPathToFilterCoefs(string pathToFilterCoefs);

        private:
            virtual bool initializeInternal(const SignalBank &input);

            string pathToFilterCoefs_;
            Real filterSpacing_, compressionCriterion_;
            Real attackTimeSTL_, releaseTimeSTL_;
            Real attackTimeLTL_, releaseTimeLTL_;
            bool sampleSpectrumUniformly_, dioticPresentation_;
            bool useDiffuseFieldResponse_, startAtWindowCentre_;
    }; 
}

#endif
