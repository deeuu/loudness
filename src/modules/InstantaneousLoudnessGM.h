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

#ifndef INSTANTANEOUSLOUDNESSGM_H
#define INSTANTANEOUSLOUDNESSGM_H

#include "../support/Module.h"

namespace loudness{

    /**
     * @class InstantaneousLoudnessGM
     *
     * @brief Given a specific loudness pattern this class computes the
     * the instantaneous loudness (integrated specific
     * loudness). 
     *
     * For input SignalBanks with one ear but diotic simulation, then you can
     * set diotic true for simple doubling of loudness. If diotic is true and
     * there are multiple ears, then both specific loudness patterns are summed
     * to a single value. If diotic is false and there are multiple ears, then
     * this module outputs one instantaneous loudness per ear.
     */
    class InstantaneousLoudnessGM : public Module
    {
    public:
 
        InstantaneousLoudnessGM(Real cParam, bool diotic);

        virtual ~InstantaneousLoudnessGM();

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual void processInternal(const SignalBank &input);
        virtual void resetInternal();

        Real cParam_;
        bool diotic_;
    };
}
#endif
