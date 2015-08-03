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
     * @brief Implementation of the DIN 45631:1991 stationary loudness model.
     * 
     * This loudness model is for processing spectra only, i.e. there is no
     * time-frequency decomposition. The input SignalBank can have multiple ears
     * and channels but only one sample per channel. This sample should
     * correspond to the intensity of a single component in normalised units.
     * Make sure that the frequencies corresponding to the input components are
     * set before initialising this model.
     *
     * If the input SignalBank used to initialise this model has one ear, then
     * the instantaneous loudness is calculated for binaural loudness. If the input
     * SignalBank has two ears, the loudness is computed for each ear according
     * to the standard, summed and then divided by two.  
     *
     * The input SignalBank is filtered by a set of 28 third octave band filters
     * (using the third order Butterworth band pass filter response given in ANSI S1.11:1986).
     *
     * OUTPUTS:
     *  - "InstantaneousLoudness"
     *
     * @todo Develop a module for transforming the main loudness to specific
     * loudness.
     *
     * REFERENCES:
     *
     * Zwicker, E., Fastl, H., Widmann, U., Kurakata, K., Kuwano, S., & Germtuo,
     * E. R. (1991). Program for calculating loudness according to DIN 45631 (ISO
     * 532B). Journal of the Acoustical Society of America, 12(1), 39–42.
     *
     * GENESIS. LOUDNESS TOOLBOX. 
     * http://genesis-acoustics.com/en/loudness_online-32.html.
     *
     * Aaron Hastings. Matlab port of the BASIC program.
     * http://www.auditory.org/mhonarc/2000/msg00498.html.
     *
     * @sa OctaveBank MainLoudnessDIN456311991 InstantaneousLoudnessDIN456311991
     */

    class StationaryLoudnessDIN456311991 : public Model
    {
        public:
            StationaryLoudnessDIN456311991(bool isPresentationDiffuseField, bool isOutputRounded);
            virtual ~StationaryLoudnessDIN456311991();

        private:
            virtual bool initializeInternal(const SignalBank &input);
            
            bool isPresentationDiffuseField_, isOutputRounded_;
    }; 
}

#endif
