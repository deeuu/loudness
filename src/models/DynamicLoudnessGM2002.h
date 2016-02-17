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

#ifndef DYNAMICLOUDNESSGM2002_H
#define DYNAMICLOUDNESSGM2002_H

#include "../support/Model.h"
#include "../support/AuditoryTools.h"

namespace loudness{

    /**
     * @class DynamicLoudnessGM2002
     *
     * @brief Implements Glasberg and Moore's (2002) dynamic loudness model.
     *
     * At present, there are three parameter sets available:
     *
     * 1. "GM2002"
     *      - The specification used by Glasberg and Moore (2002). 
     *      - This computes excitation patterns according to ANSI S3.4:2007 and will be
     *      extremely slow. 
     *      - Use setRoexBankFast(true) (default false) to speed it up.
     * 2. "Faster" 
     *      - Uses a compressed spectrum according to a 0.2 Cam criterion.
     *      - Uses a filter spacing of 0.5 Cams.
     *      - Excitation pattern is interpolated to estimate filter responses spaced at 0.1 Cams.
     *      - Uses the fast roex filter bank.
     * 3. "Recent"
     *      - Uses modified long-term loudness time-constants from Moore et al. (2003).
     *      - Uses modified equation for computing the specific loudness at very high levels (ANSI S3.4:2007).
     * 4. "FasterAndRecent"
     *      - A combination of (2) and (3).
     * 5. "WEAR2015"
     *      - Uses a compressed spectrum according to a 0.7 Cam criterion.
     *      - Uses a filter spacing of 1.25 Cams with NO interpolation.
     *      - Uses the fast roex filter bank.
     *      - Uses modified equation for computing the specific loudness at very high levels (ANSI S3.4:2007).
     *      - Time-constants from 2002 paper are used.
     *      - For faster performance, reduce the processing rate, e.g. call: setRate(500)
     *
     * The default is "FasterAndRecent". Use configureModelParameters() to switch parameter sets.
     *
     * If you want to use a time-domain filter for simulating the transmission
     * response of the outer and middle ear, such as the 4096 order FIR filter
     * used by Glasberg and Moore, then specify the path (string) to the filter
     * coefficients when constructing the object. The coefficients must be a
     * Numpy array stored as a binary file in the '.npy' format. If a file path
     * is not provided, pre-cochlear filtering is performed using the hybrid
     * filter which combines a 3rd order Butterworth high-pass filter with a frequency
     * domain weighting function (See Ward et al., 2013 and Ward et al., 2015).
     *
     * If the input SignalBank used to initialise this model has one ear, then
     * the instantaneous loudness is multiplied by two. If you don't want this,
     * call method setPresentationDiotic(false) (default is true). If the input
     * SignalBank has two ears, the default the instantaneous loudness is a sum
     * of the loudness in both left and right ears. If you want to access the
     * loudness in both left and right ears separately, call method
     * setPresentationDiotic(false). When there are two ears, the binaural
     * inhibition model proposed by Moore and Glasberg (2007) is used. If you
     * don't want this call method setInhibitSpecificLoudness(false). 
     * 
     * When using filter spacings greater than 0.1 Cams, the sampled excitation
     * pattern can be interpolated to approximate the high resolution pattern.
     * If you want this setExcitationPatternInterpolated(true). In `Faster'
     * modes, this is true.
     *
     * A peak follower can be applied to the short-term loudness using 
     * setPeakSTLFollowerUsed(true) (default is false).
     *
     * OUTPUTS:
     *  - "Excitation"
     *  - "SpecificLoudness"
     *  - "InstantaneousLoudness"
     *  - "ShortTermLoudness"
     *  - "LongTermLoudness"
     *  - "PeakShortTermLoudness" (optional)
     *
     * REFERENCES:
     *
     * Glasberg, B. R., & Moore, B. C. J. (1990). Derivation of Auditory Filter
     * Shapes From Notched-Noise Data. Hearing Research, 47, 103–138.
     *
     * Moore, B. C. J., Glasberg, B. R., & Baer, T. (1997). A Model for the
     * Prediction of Thresholds, Loudness, and Partial Loudness. Journal of the
     * Audio Engineering Society, 45(4), 224–240.
     *
     * Glasberg, B. R., & Moore, B. C. J. (2002). A Model of Loudness Applicable
     * to Time-Varying Sounds. Journal of the Audio Engineering Society, 50(5),
     * 331–342.
     *
     * Moore, B. C. J., Glasberg, B. R., & Stone, M. A. (2003). Why Are
     * Commercials so Loud ? - Perception and Modeling of the Loudness of
     * Amplitude-Compressed Speech. Journal of the Acoustical Society of America,
     * 51(12), 1123–1132.
     *
     * Glasberg, B. R., & Moore, B. C. J. (2006). Prediction of Absolute
     * Thresholds and Equal-Loudness Contours Using a Modified Loudness Model.
     * The Journal of the Acoustical Society of America, 120(2), 585–588.
     *
     * Moore, B. C. J., & Glasberg, B. R. (2007). Modeling Binaural Loudness.
     * The Journal of the Acoustical Society of America, 121(3), 1604–1612.
     *
     * ANSI. (2007). ANSI S3.4-2007. Procedure for the Computation of Loudness
     * of Steady Sounds.
     *
     * Ward, D., Athwal, C., & Kokuer, M. (2013).  An Efficient Time-Varying
     * Loudness Model.  In Proceedings of the IEEE Workshop on Applications of
     * Signal Processing to Audio and Acoustics.
     *
     * Ward, D., Enderby, S., Athwal, C., & Reiss, J. D. (2015). Real-time
     * Excitation Based Binaural Loudness Meters. In 18th International
     * International Conference on Digital Audio Effects (DAFx-15). Trondheim,
     * Norway.
     */

    class DynamicLoudnessGM2002 : public Model
    {
        public:

            /**
             * @brief Constructs a model with a path to the '.npy' file holding
             * the pre-cochlear filter coefficients.
             *
             * If no path is given, the hybrid filter will
             * perform the outer and middle ear filtering.
             */
            DynamicLoudnessGM2002(const string& pathToFilterCoefs);
            DynamicLoudnessGM2002();

            virtual ~DynamicLoudnessGM2002();

            void configureModelParameters(const string& setName);

            void setRoexBankFast(bool isRoexBankFast);

            void setExcitationPatternInterpolated(bool isExcitationPatternInterpolated);

            void setInterpolationCubic(bool isInterpolationCubic);

            void setSpectrumSampledUniformly(bool isSpectrumSampledUniformly);

            void setHoppingGoertzelDFTUsed (bool isHoppingGoertzelDFTUsed);

            void setSpectralResolutionDoubled(bool isSpectralResolutionDoubled);

            void setPresentationDiotic(bool isPresentationDiotic);

            void setBinauralInhibitionUsed(bool isBinauralInhibitionUsed);

            void setAttackTimeSTL(Real attackTimeSTL);

            void setReleaseTimeSTL(Real releaseTimeSTL);

            void setAttackTimeLTL(Real attackTimeLTL);

            void setReleaseTimeLTL(Real releaseTimeLTL);

            void setPeakSTLFollowerUsed(bool isPeakSTLFollowerUsed);

            void setOuterEarFilter(const OME::Filter& outerEarFilter);

            void setMiddleEarFilter(const OME::Filter& MiddleEarFilter);

            void setSpecificLoudnessANSIS342007(bool isSpecificLoudnessANSIS342007_);

            void setFirstSampleAtWindowCentre(bool isFirstSampleAtWindowCentre);

            void setFilterSpacingInCams(Real filterSpacingInCams);

            void setCompressionCriterionInCams(Real compressionCriterionInCams);

            void setPathToFilterCoefs(string pathToFilterCoefs);

            void configureSmoothingTimes(const string& author);

        private:
            virtual bool initializeInternal(const SignalBank &input);

            Real filterSpacingInCams_, compressionCriterionInCams_;
            Real attackTimeSTL_, releaseTimeSTL_;
            Real attackTimeLTL_, releaseTimeLTL_;
            bool isRoexBankFast_, isExcitationPatternInterpolated_, isInterpolationCubic_;
            bool isSpectrumSampledUniformly_, isHoppingGoertzelDFTUsed_;
            bool isSpectralResolutionDoubled_, isPresentationDiotic_;
            bool  isBinauralInhibitionUsed_;
            bool isSpecificLoudnessANSIS342007_, isFirstSampleAtWindowCentre_;
            bool isPeakSTLFollowerUsed_;
            string pathToFilterCoefs_;
            OME::Filter outerEarFilter_, middleEarFilter_;
    }; 
}

#endif
