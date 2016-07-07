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

#ifndef SimpleLoudness_H
#define SimpleLoudness_H

#include "../support/Module.h"

namespace loudness{

    /**
     * @class SimpleLoudness
     *
     * @brief Given a specific loudness pattern this class computes the
     * the instantaneous loudness (integrated specific loudness). 
     *
     * For input SignalBanks with one ear but simulating diotic presentation,
     * set dioticPresentation true for simple doubling of loudness.
     * If dioticPresentation is true and there are
     * two ears, then both specific loudness patterns are summed to a
     * single value. If dioticPresentation is false and there are two
     * ears, then this module outputs one instantaneous loudness value per ear
     * as well as the overall instantaneous loudness. In this case, the output
     * SignalBank will have three ears.
     */
    class SimpleLoudness : public Module
    {
    public:
 
        /** Constructs an SimpleLoudness object.
         *
         * @param cParam A scaling factor applied to the instantaneous loudness.
         * @dioticPresentation Set true for a single instantaneous loudness
         * value. For single ear SignalBanks, loudness is doubled.
         */
        SimpleLoudness(Real exponent, Real cParam, bool dioticPresentation);

        virtual ~SimpleLoudness();

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();

        Real exponent_, cParam_;
        bool dioticPresentation_;
    };
}
#endif
