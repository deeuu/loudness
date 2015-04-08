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

#ifndef BINAURALINHIBITIONMG2007_H
#define BINAURALINHIBITIONMG2007_H

#include "../support/Module.h"

namespace loudness{

    /**
     * @class BinauralInhibitionMG2007
     *
     * @brief Given two specific loudness pattern (one per ear) this module
     * implements the the binaural inhibition model proposed by Moore and
     * Glasberg (2007).
     *
     * The input SignalBank must have two ears.
     *
     * This implementation is *essentially* the same as the method described in a
     * first draft of the ISO 532-2 (2014) which is subject to change. The
     * definition of the gaussian weighting function in the original paper (Eq.
     * 1) is incorrect (typo), and the term 'g' is specified in normalised
     * units, but I believe it should be an absolute deviation from the centre
     * frequency (in Cams) as given in the ISO document. Thus, a single kernel
     * can be used for the convolution, otherwise the shape of the weigthing
     * function would be frequency dependent.
     *
     * REFERENCES:
     *
     * Moore, B. C. J., & Glasberg, B. R. (2007). Modeling Binaural Loudness. The
     * Journal of the Acoustical Society of America, 121(3), 1604–1612.
     *
     * ISO. (2014). First ISO/CD 532-2 “Acoustics - Method for calculating
     * loudness - Part 2: Moore-Glasberg method.”
     */

    class BinauralInhibitionMG2007 : public Module
    {
    public:
 
        /** Constructs an BinauralInhibitionMG2007 object. */
        BinauralInhibitionMG2007();

        virtual ~BinauralInhibitionMG2007();

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual void processInternal(const SignalBank &input);
        virtual void resetInternal();

        RealVec gaussian_;
    };
}
#endif
