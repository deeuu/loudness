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

#include "OctaveBank.h"

namespace loudness{

    OctaveBank::OctaveBank(int order, int nBandsToRemoveFromEnd, bool isThirdOctave, bool isOutputInDecibels) :
        Module("OctaveBank"),
        order_ (order),
        nBandsToRemoveFromEnd_ (nBandsToRemoveFromEnd),
        isThirdOctave_ (isThirdOctave),
        isOutputInDecibels_ (isOutputInDecibels)
    {
        centreFreqs_ = {25, 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 250, 315,
            400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000,
            6300, 8000, 10000, 12500, 16000, 20000};

        if ((nBandsToRemoveFromEnd_ > 0) && (nBandsToRemoveFromEnd_ < 30))
        {
            centreFreqs_.erase (centreFreqs_.end() - nBandsToRemoveFromEnd_,
                                centreFreqs_.end());
        }
                                
    }

    OctaveBank::~OctaveBank() {}

    void OctaveBank::setCentreFreqs (RealVec centreFreqs)
    {
        centreFreqs_ = centreFreqs;
    }

    bool OctaveBank::initializeInternal(const SignalBank &input)
    {
        if (centreFreqs_.size() < 1)
        {
            LOUDNESS_ERROR (name_
                            << ": Must have at least 1 centre frequency"
                            << " to generate the filter response(s).");
            return 0;
        }

        // Reference Q
        Real b = 1.0;
        if (isThirdOctave_)
            b = 1.0 / 3.0;
        Real qRef = 1.0 / (pow(2, (b / 2.0)) - pow(2, (-b / 2.0)));

        LOUDNESS_DEBUG(name_ << ": Reference Q: " << qRef);

        // Calculation of design bandwidth for Butterworth filter
        Real c = PI / (2.0 * order_);
        Real qDes = qRef * c / std::sin (c);
        exponent_ = 2.0 * order_;
        qDesExponentiated_ = pow (qDes, exponent_);

        LOUDNESS_DEBUG(name_ << 
                ": Design Q for Butterworth filter of order " 
                << order_ <<
                ": " << qDes);

        // Output SignalBank
        output_.initialize (input.getNSources(),
                            input.getNEars(),
                            centreFreqs_.size(),
                            1,
                            input.getFs());
        output_.setCentreFreqs (centreFreqs_);
        output_.setFrameRate (input.getFrameRate());

        return 1;
    }

    void OctaveBank::processInternal(const SignalBank &input)
    {
        for (int src = 0; src < input.getNSources(); ++src)
        {
            for (int ear = 0; ear < input.getNEars(); ++ear)
            {
                const Real* inputSpectrum = input
                                            .getSingleSampleReadPointer
                                            (src, ear, 0);
                Real* output = output_
                               .getSingleSampleWritePointer
                               (src, ear, 0);

                for (uint i = 0; i < centreFreqs_.size(); ++i)
                {
                    Real filterOutput = 0.0;
                    Real fm = centreFreqs_[i];

                    for (int j = 0; j < input.getNChannels(); ++j)
                    {
                        if (inputSpectrum[j] > 1e-15)
                        {
                            Real f = input.getCentreFreq (j);
                            Real g = f / fm - fm / f;
                            Real aDes = 1 + qDesExponentiated_ * pow (g, exponent_);
                            filterOutput += inputSpectrum[j] / aDes;
                        }
                    }

                    if (isOutputInDecibels_)
                         filterOutput = powerToDecibels (filterOutput);

                    output[i] = filterOutput;
                }
            }
        }
    }

   void OctaveBank::resetInternal(){};
}
