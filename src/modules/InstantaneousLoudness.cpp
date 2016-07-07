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

#include "InstantaneousLoudness.h"
#include "../support/AuditoryTools.h"

namespace loudness{

    InstantaneousLoudness::InstantaneousLoudness(Real cParam, bool dioticPresentation) :
        Module("InstantaneousLoudness"),
        cParam_(cParam),
        dioticPresentation_(dioticPresentation)
    {
        LOUDNESS_DEBUG(name_ << ": Constructed.");
    }

    InstantaneousLoudness::~InstantaneousLoudness()
    {};

    bool InstantaneousLoudness::initializeInternal(const SignalBank &input)
    {
        LOUDNESS_ASSERT(input.getNChannels() > 1,
                name_ << ": Insufficient number of input channels.");
        LOUDNESS_ASSERT(isPositiveAndLessThanUpper(input.getNEars(), 3),
                name_ << ": A human has no more than two ears.");

        //assumes uniformly spaced ERB filters
        Real camStep = input.getChannelSpacingInCams(); 
        cParam_ *= camStep;
        LOUDNESS_ASSERT(camStep > 0, 
                name_ << ": Channel spacing (in Cam units) not set.");
        LOUDNESS_DEBUG(name_ << ": Filter spacing (Cams): " << camStep);

        //configure output SignalBank depending on number of ears and dioticPresentation
        if (input.getNEars() == 1)
        {
            if (dioticPresentation_)
            {
                cParam_ *= 2;
                LOUDNESS_DEBUG(name_ <<
                        ": diotic presentation presentation; loudness will be multiplied by 2.");
            }
            output_.initialize (input.getNSources(), 1, 1, 1, input.getFs());
        }
        else if (dioticPresentation_)
        {
            LOUDNESS_DEBUG(name_ <<
                    ": diotic presentation presentation; loudness will be summed across ears.");
            output_.initialize (input.getNSources(), 1, 1, 1, input.getFs());
        }
        else
        {
            LOUDNESS_DEBUG(name_ 
                    << ": dichotic presentation presentation;" 
                    << " loudness output in each ear, plus overall loudness (3 ear output).");
            output_.initialize (input.getNSources(),
                               input.getNEars() + 1, 1, 1, input.getFs());
        }
        output_.setFrameRate (input.getFrameRate());

        return 1;
    }

    void InstantaneousLoudness::processInternal(const SignalBank &input)
    {       
        for (int src = 0; src < input.getNSources(); ++src)
        {
            Real earIL = 0.0, overallIL = 0.0;
            for (int ear = 0; ear < input.getNEars(); ++ear)
            {
                const Real* inputSpecificLoudness = input
                                                    .getSingleSampleReadPointer
                                                    (src, ear, 0);

                // sum loudness over all auditory filters
                for (int chn = 0; chn < input.getNChannels(); ++chn)
                    earIL += inputSpecificLoudness[chn];
                
                earIL *= cParam_;

                // if not dioticPresentation then output loudness in each ear
                if (!dioticPresentation_)
                    output_.setSample (src, ear, 0, 0, earIL);

                //sum and reset
                overallIL += earIL;
                earIL = 0.0;
            }

            if (dioticPresentation_)
                output_.setSample (src, 0, 0, 0, overallIL);
            else if (output_.getNEars() == 3)
                output_.setSample (src, 2, 0, 0, overallIL);
        }
    }

    //output SignalBanks are cleared so not to worry about filter state
    void InstantaneousLoudness::resetInternal()
    {
    }
}
