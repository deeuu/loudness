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

#ifndef StationaryLOUDNESSANSIS342007_H
#define StationaryLOUDNESSANSIS342007_H

#include "../support/Model.h"
#include "../support/AuditoryTools.h"
#include <string>

namespace loudness{

    /**
     * @brief Implementation of the ANSI S3.4:2007 stationary loudness model.
     * 
     * This loudness model is for processing spectra only, i.e. there is no
     * time-frequency decomposition. The input SignalBank can have multiple ears
     * and channels but only one sample per channel. This sample should
     * correspond to the intensity of a single component in normalised units.
     * Make sure that the frequencies corresponding to the input components are
     * set before initialising this model.
     *
     * If the input SignalBank used to initialise this model has one ear, then
     * the instantaneous loudness is multiplied by two. If you don't want this,
     * call the method setDioticPresentation(false) (default is true). If the input
     * SignalBank has two ears, the default the instantaneous loudness is a sum
     * of the loudness in both left and right ears. If you want to access the
     * loudness in both left and right ears separately, call method
     * setDioticPresentation(false). When there are two ears, the binaural
     * inhibition model proposed by Moore and Glasberg (2007) is used. If you
     * don't want this call method setInhibitSpecificLoudness(false). 
     *
     * OUTPUTS:
     *  - "Excitation"
     *  - "SpecificLoudness"
     *  - "InstantaneousLoudness"
     *
     * It should be noted that this model makes use of polynomials to
     * approximate the variables involved in the specific loudness calculations,
     * rather than interpolation as used in the standard. See
     * SpecificLoudnessANSIS342007 to see how the specific loudness parameters
     * are computed.
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
     */

    class StationaryLoudnessANSIS342007 : public Model
    {
        public:
            StationaryLoudnessANSIS342007();
            virtual ~StationaryLoudnessANSIS342007();

            void setPresentationDiotic(bool isPresentationDiotic);

            void setBinauralInhibitionUsed(bool isBinauralInhibitionUsed);

            void setOuterEarFilter(const OME::Filter outerEarFilter);

            void setfilterSpacingInCams(Real filterSpacingInCams);

            void setSpecificLoudnessANSIS342007(bool isSpecificLoudnessANSIS342007_);
            
        private:
            virtual bool initializeInternal(const SignalBank &input);

            Real filterSpacingInCams_;
            bool isPresentationDiotic_, isBinauralInhibitionUsed_;
            bool isSpecificLoudnessANSIS342007_;
            OME::Filter outerEarFilter_;
    }; 
}

#endif
