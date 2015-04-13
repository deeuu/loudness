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
     * If the input SignalBank used to initialise this model has one ear, then
     * the instantaneous loudness is multiplied by two. If you don't want this,
     * call method setDioticPresentation(false) (default is true). If the input
     * SignalBank has two ears, the default the instantaneous loudness is a sum
     * of the loudness in both left and right ears. If you want to access the
     * loudness in both left and right ears seperately, call method
     * setDioticPresentation(false). When there are two ears, the binaural
     * inhibition model proposed by Moore and Glasberg (2007) is used. If you
     * don't want this call method setInhibitSpecificLoudness(false). Note that
     * because this model does not have an explicit `specific loudness' stage,
     * the excitation pattern is scaled to give units of sones rather than
     * power. If you don't want this, call method setOutputSpecificLoudness
     * (false) (default is true). In this case, binaural inhibition will not be
     * implemented.
     *
     * When using filter spacings greater than 0.1 Cams, the sampled excitation
     * pattern can be interpolated to approximate the high resolution pattern.
     * If you want this setExcitationPatternInterpolated(true); In mode `faster'
     * this is true;
     *
     * REFERENCES:
     *
     * Moore, B. C. J., & Glasberg, B. R. (2007). Modeling Binaural Loudness. The
     * Journal of the Acoustical Society of America, 121(3), 1604–1612.
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

            void setSpectrumSampledUniformly(bool isSpectrumSampledUniformly);

            void setExcitationPatternInterpolated(bool isExcitationPatternInterpolated);

            void setPresentationDiotic(bool isPresentationDiotic);

            void setBinauralInhibitionUsed(bool isBinauralInhibitionUsed);

            void setResponseDiffuseField(bool isResponseDiffuseField);

            void setFirstSampleAtWindowCentre(bool isFirstSampleAtWindowCentre);

            void setSpecificLoudnessOutput(bool isSpecificLoudnessOutput);

            void setFilterSpacingInCams(Real filterSpacingInCams);

            void setCompressionCriterionInCams(Real compressionCriterionInCams);

            void setPathToFilterCoefs(string pathToFilterCoefs);

        private:
            virtual bool initializeInternal(const SignalBank &input);

            string pathToFilterCoefs_;
            Real filterSpacingInCams_, compressionCriterionInCams_;
            Real attackTimeSTL_, releaseTimeSTL_;
            Real attackTimeLTL_, releaseTimeLTL_;
            bool isSpectrumSampledUniformly_, isExcitationPatternInterpolated_, isPresentationDiotic_;
            bool isSpecificLoudnessOutput_, isBinauralInhibitionUsed_;
            bool isResponseDiffuseField_, isFirstSampleAtWindowCentre_;
    }; 
}

#endif
