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

#ifndef StationaryLOUDNESSDIN456311991_H
#define StationaryLOUDNESSDIN456311991_H

#include "../support/Model.h"

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
     * REFERENCES:
     *
     */

    class StationaryLoudnessDIN456311991 : public Model
    {
        public:
            StationaryLoudnessDIN456311991();
            virtual ~StationaryLoudnessDIN456311991();

        private:
            virtual bool initializeInternal(const SignalBank &input);
    }; 
}

#endif
