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
     * This loudness model is for processing spectra only, i.e., there is no
     * time-frequency decomposition. The input SignalBank can have
     * multiple ears and channels but only one sample per channel which
     * specifies the intensity of a single component. Make sure that the centre
     * frequencies corresponding to each input component are set.
     *
     * If the input SignalBank used to initialise this model has one ear, then
     * the instantaneous loudness is multiplied by two. If you don't want this,
     * call method setDioticPresentation(false) (default is true). If the input
     * SignalBank has two ears, the default the instantaneous loudness is a sum
     * of the loudness in both left and right ears. If you want to access the
     * loudness in both left and right ears separately, call method
     * setDioticPresentation(false). When there are two ears, the binaural
     * inhibition model proposed by Moore and Glasberg (2007) is used. If you
     * don't want this call method setInhibitSpecificLoudness(false). 
     *
     * REFERENCES:
     * Chen, Z., Hu, G., Glasberg, B. R., & Moore, B. C. J. (2011). A new method
     * of calculating auditory excitation patterns and loudness for steady
     * sounds. Hearing Research, 282(1-2), 204–15.
     *
     */
    class StationaryLoudnessCHGM2011 : public Model
    {
        public:
            StationaryLoudnessCHGM2011();
            virtual ~StationaryLoudnessCHGM2011();

            void setPresentationDiotic(bool isPresentationDiotic);

            void setBinauralInhibitionUsed(bool isBinauralInhibitionUsed);

            void setOuterEarType(const OME::Filter outerEarType);

            void setfilterSpacingInCams(Real filterSpacingInCams);

            void setSpecificLoudnessOutput(bool isSpecificLoudnessOutput);

        private:
            virtual bool initializeInternal(const SignalBank &input);

            Real filterSpacingInCams_;
            bool isPresentationDiotic_, isBinauralInhibitionUsed_;
            bool isSpecificLoudnessOutput_;
            OME::Filter outerEarType_;
    }; 
}

#endif
