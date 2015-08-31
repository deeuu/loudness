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

#ifndef StationaryLoudnessCHGM2011_H
#define StationaryLoudnessCHGM2011_H

#include "../support/Model.h"
#include "../support/AuditoryTools.h"
#include <string>

namespace loudness{

    /**
     * @brief Implementation of the Chen et al 2011 stationary loudness model.
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
     * call the method setPresentationDiotic(false) (default is true). If the input
     * SignalBank has two ears, the default the instantaneous loudness is a sum
     * of the loudness in both left and right ears. If you want to access the
     * loudness in both left and right ears separately, call method
     * setPresentationDiotic(false). When there are two ears, the binaural
     * inhibition model proposed by Moore and Glasberg (2007) is used. If you
     * don't want this call method setInhibitSpecificLoudness(false). Note that
     * because this model does not have a separate `specific loudness' stage,
     * the excitation pattern is scaled to give units of sones rather than
     * power. If you don't want this, call method setOutputSpecificLoudness
     * (false) (default is true). In this case, binaural inhibition will not be
     * implemented.
     *
     * OUTPUTS:
     *  - "SpecificLoudness"
     *  - "InstantaneousLoudness"
     *
     * REFERENCES:
     *
     * Chen, Z., Hu, G., Glasberg, B. R., & Moore, B. C. J. (2011). A new method
     * of calculating auditory excitation patterns and loudness for steady
     * sounds. Hearing Research, 282(1-2), 204–15.
     *
     * Moore, B. C. J., & Glasberg, B. R. (2007). Modeling Binaural Loudness.
     * The Journal of the Acoustical Society of America, 121(3), 1604–1612.
     *
     * @sa DoubleRoexBank
     *
     */
    class StationaryLoudnessCHGM2011 : public Model
    {
        public:
            StationaryLoudnessCHGM2011();
            virtual ~StationaryLoudnessCHGM2011();

            void setPresentationDiotic(bool isPresentationDiotic);

            void setBinauralInhibitionUsed(bool isBinauralInhibitionUsed);

            void setOuterEarFilter(const OME::Filter outerEarFilter);

            void setfilterSpacingInCams(Real filterSpacingInCams);

            void setSpecificLoudnessOutput(bool isSpecificLoudnessOutput);

        private:
            virtual bool initializeInternal(const SignalBank &input);

            Real filterSpacingInCams_;
            bool isPresentationDiotic_, isBinauralInhibitionUsed_;
            bool isSpecificLoudnessOutput_;
            OME::Filter outerEarFilter_;
    }; 
}

#endif
