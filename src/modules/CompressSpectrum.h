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

#ifndef COMPRESSSPECTRUM_H
#define COMPRESSSPECTRUM_H

#include "../support/Module.h"

namespace loudness{

    /**
     * @class CompressSpectrum
     *
     * @brief Reduces the number of spectral components by averaging the
     * spectrum using a rectangular window.
     *
     * The summation criterion (alpha) determines the bandwidth of the window;
     * the algorithms keeps components within alpha spacing on a Cam
     * (perceptual) scale. Generally, this can only be achieved at mid-hi
     * frequencies where the spectrum is dense.
     *
     * This is achieved by examining the derivative of the input frequencies (which
     * are assumed to be linearly distributed) on a Cam frequency scale and
     * grouping components according to the compression criterion. 
     *
     * There are other solutions to the problem such as adding alpha and
     * counting the number of bins below, but that approach fails to satisfy the
     * criterion for alpha > 2. Anyway, this approach only costs about +2 bins
     * more on average.
     *
     * @todo clean code.
     *
     * @author Dominic Ward
     */
    class CompressSpectrum : public Module
    {

    public:

        CompressSpectrum(Real alpha=0.2);

        virtual ~CompressSpectrum();

    private:
        virtual bool initializeInternal(const SignalBank &input);

        virtual void processInternal(const SignalBank &input);

        virtual void resetInternal();

        vector<int> upperBandIdx_;
        Real alpha_;
    };
}

#endif
