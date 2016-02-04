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

#ifndef MainLoudnessDIN456311991_H
#define MainLoudnessDIN456311991_H

#include "../support/Module.h"

namespace loudness{

    /**
     * @class MainLoudnessDIN456311991
     *
     * @brief Given a set of 28 third-octave band levels, this module computes
     * the main loudness of each band.
     *
     * The input SignalBank should have 28 channels with the following centre
     * frequencies:
     *    {25, 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315, 400, 500,
     *    630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000,
     *    10000, 12500}
     * The corresponding samples should be in dB SPL.
     *
     * Two ears are supported, but the main binaural loudness is calculated for
     * each ear separately. This is so that the output SignalBank can then be
     * passed to InstantaneousLoudnessDIN456311991 which then sums the total
     * loudness in each ear and divides the sum by two.
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
     * @sa InstantaneousLoudnessDIN456311991
     */
 
    class MainLoudnessDIN456311991 : public Module
    {

    public:

        enum OuterEarFilter{
                NONE,
                FREEFIELD,
                DIFFUSEFIELD,
            };

        MainLoudnessDIN456311991 (const OuterEarFilter& outerEarType = OuterEarFilter::FREEFIELD);
        virtual ~MainLoudnessDIN456311991();

    private:

        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();
        
        OuterEarFilter outerEarType_;
        RealVecVec dLL_;
        RealVec rAP_, a0_, dDF_, lTQ_, dCB_;
    };
}

#endif
