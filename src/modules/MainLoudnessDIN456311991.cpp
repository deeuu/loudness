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

#include "MainLoudnessDIN456311991.h"
#include "../support/AuditoryTools.h"

namespace loudness{

    MainLoudnessDIN456311991::MainLoudnessDIN456311991(const OuterEarFilter& outerEarType) :
        Module("MainLoudnessDIN456311991"),
        outerEarType_ (outerEarType),
        dLL_ (11, RealVec(8, 0.0)),
        rAP_ (8, 0.0),
        a0_ (20, 0.0),
        dDF_ (20, 0.0),
        lTQ_ (20, 0.0)
    {

        /* Reduction of 1/3 octave band levels at low frequencies according to
         * equal loudness contours within the eight ranges defined by RAP */
        dLL_[0] = {-32, -29, -27, -25, -23, -20, -18, -15};
        dLL_[1] = {-24, -22, -19, -17, -16, -14, -12, -10};
        dLL_[2] = {-16, -15, -14, -12, -11, -10, -9, -8};
        dLL_[3] = {-10, -10, -9, -9, -7, -6, -6,-4};
        dLL_[4] = {-5, -4, -4, -3, -3, -3, -2, -2};
        dLL_[5] = {0, 0, 0, 0, 0, 0, 0, 0};
        dLL_[6] = {-7, -7, -6, -5, -4, -4, -3, -3};
        dLL_[7] = {-3, -2, -2, -2, -1, -1, -1, -1};
        dLL_[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        dLL_[9] = {-2, -2, -2, -2, -1, -1, -1, -1};
        dLL_[10] = {0, 0, 0, 0, 0, 0, 0, 0};

        /* Ranges of 1/3 octave band levels for correction at low frequencies
         * according to equal loudness contours */
        rAP_ = {45,55,65,71,80,90,100,120};

        /* Correction of levels according to the transmission characteristics of
         * the ear: Fastl and Zwicker p.225: a0 represents the transmission
         * between the freefield and our hearing system. This is attenuation
         * (dB) as a function of bark. */
        a0_ = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -0.5, -1.6,
               -3.2, -5.4, -5.6, -4, -1.5, 2, 5, 12};

        // Level differences between free and diffuse sound fields
        dDF_ = {0, 0, 0.5, 0.9, 1.2, 1.6, 2.3, 2.8, 3, 2, 0, -1.4, -2, -1.9, -1,
                0.5, 3, 4, 4.3, 4}; 

        /* Critical band rate level at absolute threshold without taking into
         * account the transmission characteristics of the ear */
        lTQ_ = {30,18,12,8,7,6,5,4,3,3,3,3,3,3,3,3,3,3,3,3};

        /* Adaptation of 1/3 octave band levels to the corresponding critical
         * band levels */
        dCB_ = {-0.25, -0.6, -0.8, -0.8, -0.5, 0, 0.5, 1.1, 1.5, 1.7, 1.8, 1.8,
                1.7, 1.6, 1.4, 1.2, 0.8, 0.5, 0, -0.5};

    }

    MainLoudnessDIN456311991::~MainLoudnessDIN456311991() {}

    bool MainLoudnessDIN456311991::initializeInternal(const SignalBank &input)
    {
        if (input.getNChannels() != 28)
        {
            LOUDNESS_ERROR(name_ << ": Input should have 28 channels");
            return 0;
        }
        else
        {
            RealVec expectedCentreFreqs = {25, 31.5, 40, 50, 63, 80, 100, 125,
                160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000,
                2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500};

            if (input.getCentreFreqs() != expectedCentreFreqs)
            {
                LOUDNESS_ERROR(name_ << ": Centre frequencies of the input SignalBank are incorrect");
                return 0;
            }
        }

        // Output SignalBank
        output_.initialize (input.getNEars(), 21, 1, input.getFs());
        output_.setFrameRate (input.getFrameRate());
        /* Centre frequencies not given but upper limits of bark bands are
         * given, so apply -0.5 bark? This isn't needed for subsequent
         * processing anyway. This could be incorrect: */
        RealVec barkLimits = {0.9, 1.8, 2.8, 3.5, 4.4, 5.4, 6.6, 7.9, 9.2, 10.6,
            12.3, 13.8, 15.2, 16.7, 18.1, 19.3, 20.6, 21.8, 22.7, 23.6, 24.0};
        for (uint i = 0; i < barkLimits.size(); ++i)
        {
            Real fc = barkToHertz (barkLimits[i] - 0.5);
            output_.setCentreFreq (i, fc);
        }

        return 1;
    }

    void MainLoudnessDIN456311991::processInternal(const SignalBank &input)
    {
        for (int ear = 0; ear < input.getNEars(); ++ear)
        {
            const Real* inputLevels = input.getSingleSampleReadPointer (ear, 0);
            Real* mainLoudness = output_.getSingleSampleWritePointer (ear, 0);
            
            // Levels only acceptable between -60 and 120 dB SPL
            RealVec thirdOctaveLevels(input.getNChannels(), -60.0);
            for (int i = 0; i < input.getNChannels(); ++i)
            {
                if (inputLevels[i] > 120)
                    thirdOctaveLevels[i] = 120;
                else if (inputLevels[i] > -60)
                    thirdOctaveLevels[i] = inputLevels[i];
            }

            // Determination of levels (LCB) within the first three critical bands
            int nCriticalBands = 20;
            RealVec lE(nCriticalBands, 0.0);
            Real tI = 0.0;
            for (uint i = 0; i < dLL_.size(); ++i) // size = 11
            {
                int j = 0;
                while ((thirdOctaveLevels[i] > (rAP_[j] - dLL_[i][j])) && (j < 7))
                    j++;
                Real xP = thirdOctaveLevels[i] + dLL_[i][j];
                tI += decibelsToPower (xP);

                if (i == 5) // Sum of 6 third octave bands from 25 Hz to 80 Hz
                {
                    lE[0] = powerToDecibels (tI);
                    tI = 0.0;
                }
                else if (i == 8) // Sum of 3 third octave bands from 100 Hz to 160 Hz
                {
                    lE[1] = powerToDecibels (tI);
                    tI = 0.0;
                }
                else if (i == 10) // Sum of 2 third octave bands from 200 Hz to 250 Hz
                {
                    lE[2] = powerToDecibels (tI);
                }
            }

            // Calculation of main loudness
            for (int i = 0; i < nCriticalBands; ++i)
            {
                // First three bands derived from summed third octave bands
                if (i > 2)
                    lE[i] = thirdOctaveLevels[i + 8];

                if (outerEarType_ == OuterEarFilter::FREEFIELD)
                    lE[i] = lE[i] - a0_[i];
                else if (outerEarType_ == OuterEarFilter::DIFFUSEFIELD)
                    lE[i] = lE[i] - a0_[i] + dDF_[i];

                mainLoudness[i] = 0.0;

                if (lE[i] > lTQ_[i])
                {
                    lE[i] = lE[i] - dCB_[i];
                    Real mP1 = 0.0635 * std::pow (10, 0.025 * lTQ_[i]);
                    Real powerRatio = decibelsToPower (lE[i] - lTQ_[i]);
                    Real s = 0.25;
                    Real mP2 = std::pow (1 - s + s * powerRatio, 0.25) - 1;
                    mainLoudness[i] = mP1 * mP2;
                    mainLoudness[i] = (mainLoudness[i] < 0.0) ? 0.0 : mainLoudness[i];
                }
            }
            mainLoudness[nCriticalBands] = 0.0;

            /* correction of specific loudness within the first critical band
             * taking into account the dependence of absolute threshold within
             * the band */
            Real kORRY = 0.4 + 0.32 * std::pow(mainLoudness[0], 0.2);
            mainLoudness[0] = kORRY > 1 ? mainLoudness[0] : mainLoudness[0] * kORRY;
        }
    }

   void MainLoudnessDIN456311991::resetInternal(){};
}
