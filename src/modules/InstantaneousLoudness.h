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

#ifndef INSTANTANEOUSLOUDNESS_H
#define INSTANTANEOUSLOUDNESS_H

#include "../support/Module.h"

namespace loudness{

    /**
     * @class InstantaneousLoudness
     *
     * @brief Given a specific loudness pattern this class computes the
     * the instantaneous loudness (integrated specific loudness). 
     *
     * For input SignalBanks with one ear but simulating diotic presentation,
     * then you can set dioticPresentation true for simple
     * doubling of loudness. If dioticPresentation is true and there are
     * multiple ears, then both specific loudness patterns are summed to a
     * single value. If dioticPresentation is false and there are multiple
     * ears, then this module outputs one instantaneous loudness value per ear.
     */
    class InstantaneousLoudness : public Module
    {
    public:
 
        /** Constructs an InstantaneousLoudness object.
         *
         * @param cParam A scaling factor applied to the instantaneous loudness.
         * @dioticPresentation Set true for a single instantaneous loudness
         * value. For single ear SignalBanks, loudness is doubled.
         */
        InstantaneousLoudness(Real cParam, bool dioticPresentation);

        virtual ~InstantaneousLoudness();

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();

        Real cParam_;
        bool dioticPresentation_;
    };
}
#endif
