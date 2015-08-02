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

#ifndef InstantaneousLoudnessDIN45631_H
#define InstantaneousLoudnessDIN45631_H

#include "../support/Module.h"

namespace loudness{

    /**
     * @class InstantaneousLoudnessDIN45631
     *
     * @brief Calculates the total loudness from the main loudness according to
     * DIN 45631 (1990).
     *
     * The input SignalBank should have 21 channels corresponding to the
     * approximated critical bands derived from third-octave filter bands.
     * If the input SignalBank has two ears, the loudness in each is computed
     * according to the standard and the sum of loudnesses is divided by two.
     * The output SignalBank of this module has one ear, one channel and one sample.
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
     * @sa MainLoudnessDIN45631
     */
 
    class InstantaneousLoudnessDIN45631 : public Module
    {

    public:

        InstantaneousLoudnessDIN45631 ();
        virtual ~InstantaneousLoudnessDIN45631();

    private:

        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();
        
        RealVec zUP_, rNS_;
        RealVecVec uSL_;
    };
}

#endif
