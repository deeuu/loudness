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

#include "HighpassSpectrum.h"

namespace loudness{

    HighpassSpectrum::HighpassSpectrum(Real fc, Real gainDB, Real slope) :
        Module("HighpassSpectrum"),
        fc_ (fc),
        gainDB_ (gainDB),
        slope_ (slope)
    {}

    HighpassSpectrum::~HighpassSpectrum() {}

    bool HighpassSpectrum::initializeInternal(const SignalBank &input)
    {

        for (int chn = 0; chn < input.getNChannels(); chn++)
        {
            Real g = (input.getCentreFreq (chn) - fc_) / fc_;
            Real weight = gainDB_ * -std::exp (slope_ * -g);
            weights_.push_back (std::pow (10.0, weight / 10.0));
        }

        //set output SignalBank
        output_.initialize (input);

        return 1;
    }

    void HighpassSpectrum::processInternal(const SignalBank &input)
    {
        for (int src = 0; src < input.getNSources(); ++src)
        {
            for (int ear = 0; ear < input.getNEars(); ++ear)
            {
                const Real* inputSpectrum = input.getSingleSampleReadPointer
                                            (src, ear, 0);
                Real* outputSpectrum = output_.getSingleSampleWritePointer
                                       (src, ear, 0);

                for (int chn = 0; chn < input.getNChannels(); ++chn)
                    outputSpectrum[chn] = inputSpectrum[chn] * weights_[chn];
            }
        }
    }

    void HighpassSpectrum::resetInternal(){};
}
