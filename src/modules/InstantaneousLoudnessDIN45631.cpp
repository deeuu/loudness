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

#include "InstantaneousLoudnessDIN45631.h"

namespace loudness{

    InstantaneousLoudnessDIN45631::InstantaneousLoudnessDIN45631() :
        Module("InstantaneousLoudnessDIN45631"),
        zUP_ (21, 0.0),
        rNS_ (21, 0.0),
        uSL_ (18, RealVec(8, 0.0))
    {
        /* Upper limits of approximated critical bands in terms of critical band
         * rate */
        zUP_ = {0.9, 1.8, 2.8, 3.5, 4.4, 5.4, 6.6, 7.9, 9.2, 10.6, 12.3, 13.8,
                15.2, 16.7, 18.1, 19.3, 20.6, 21.8, 22.7, 23.6, 24};
        /* Range of specific loudness for the determination of the steepness of
         * the upper slopes in the specific loudness - critical band rate pattern */
        rNS_ = {21.5, 18, 15.1, 11.5, 9, 6.1, 4.4, 3.1, 2.13, 1.36, 0.82, 0.42,
            0.3, 0.22, 0.15, 0.1, 0.035, 0};

        /* Steepness of the upper slopes in the specific loudness - critical
         * band rate pattern for the ranges RNS as a function of the number of
         * the critical band */
        uSL_[0] = {13,8.2,6.3,5.5,5.5,5.5,5.5,5.5};
        uSL_[1] = {9,7.5,6,5.1,4.5,4.5,4.5,4.5};
        uSL_[2] = {7.8,6.7,5.6,4.9,4.4,3.9,3.9,3.9};
        uSL_[3] = {6.2,5.4,4.6,4.0,3.5,3.2,3.2,3.2};
        uSL_[4] = {4.5,3.8,3.6,3.2,2.9,2.7,2.7,2.7};
        uSL_[5] = {3.7,3.0,2.8,2.35,2.2,2.2,2.2,2.2};
        uSL_[6] = {2.9,2.3,2.1,1.9,1.8,1.7,1.7,1.7};
        uSL_[7] = {2.4,1.7,1.5,1.35,1.3,1.3,1.3,1.3};
        uSL_[8] = {1.95,1.45,1.3,1.15,1.1,1.1,1.1,1.1};
        uSL_[9] = {1.5,1.2,0.94,0.86,0.82,0.82,0.82,0.82};
        uSL_[10] = {0.72,0.67,0.64,0.63,0.62,0.62,0.62,0.62};
        uSL_[11] = {0.59,0.53,0.51,0.5,0.42,0.42,0.42,0.42};
        uSL_[12] = {0.4,0.33,0.26,0.24,0.24,0.22,0.22,0.22};
        uSL_[13] = {0.27,0.21,0.2,0.18,0.17,0.17,0.17,0.17};
        uSL_[14] = {0.16,0.15,0.14,0.12,0.11,0.11,0.11,0.11};
        uSL_[15] = {0.12,0.11,0.1,0.08,0.08,0.08,0.08,0.08};
        uSL_[16] = {0.09,0.08,0.07,0.06,0.06,0.06,0.06,0.05};
        uSL_[17] = {0.06,0.05,0.03,0.02,0.02,0.02,0.02,0.02};
    }

    InstantaneousLoudnessDIN45631::~InstantaneousLoudnessDIN45631() {}

    bool InstantaneousLoudnessDIN45631::initializeInternal(const SignalBank &input)
    {
        if (input.getNChannels() != 21)
        {
            LOUDNESS_ERROR(name_ << ": Input SignalBank should have 21 channels");
            return 0;
        }

        // Output SignalBank
        output_.initialize (1, 1, 1, input.getFs());
        output_.setFrameRate (input.getFrameRate());

        return 1;
    }

    void InstantaneousLoudnessDIN45631::processInternal(const SignalBank &input)
    {
        Real nTot = 0.0;
        for (int ear = 0; ear < input.getNEars(); ++ear)
        {
            const Real* mainLoudness = input.getSingleSampleReadPointer (ear, 0);

            Real n = 0.0;
            Real z1 = 0.0;
            Real z2 = 0.0;
            Real n1 = 0.0;
            Real n2 = 0.0;
            int iZ = 0;
            int j = 0;
            Real z = 0.1;
            Real barkStep = 0.1;

            for (int i = 0; i < input.getNChannels(); ++i)
            {
                Real zUP = zUP_[i] + 0.0001;
                int iG = i - 1;

                // Steepness of upper slope (USL) for bands above 8th one are identical
                if (iG > 7)
                    iG = 7;

                while (z1 < zUP)
                {
                    if (n1 <= mainLoudness[i])
                    {
                        /* Contribution of unmasked main loudness to total
                         * loudness and calculation of values */
                        if (n1 < mainLoudness[i])
                        {
                            j = 0;
                            /* Determination of the number j corr to the range
                             * of specific loudness */
                            while ((rNS_[j] > mainLoudness[i]) && (j < 17))
                                j += 1;
                        }

                        z2 = zUP;
                        n2 = mainLoudness[i];
                        n += n2 * (z2 - z1);
                        Real k = z;

                        while (k <= z2)
                        {
                            //NS[IZ] = N2
                            iZ += 1;
                            k += barkStep;
                        }
                        z = k;
                    }
                    else
                    {
                        /* If N1 > NM(i) decision whether the critical band in
                         * question is completely or partly masked by accessory
                         * loudness */
                        n2 = rNS_[j];
                        if (n2 < mainLoudness[i])
                            n2 = mainLoudness[i];

                        Real dZ = (n1 - n2) / uSL_[j][iG];
                        z2 = z1 + dZ;

                        if (z2 > zUP)
                        {
                            z2 = zUP;
                            dZ = z2 - z1;
                            n2 = n1 - dZ * uSL_[j][iG];
                        }

                        n += dZ * (n1 + n2) / 2.0;
                        Real k = z;

                        while (k <= z2)
                        {
                            //NS[IZ] = N1 - (k - Z1) * USL[j,IG]
                            iZ += 1;
                            k += barkStep;
                        }
                        z = k;
                    }

                    while ((n2 <= rNS_[j]) && (j < 17))
                        j += 1;

                    if ((n2 <= rNS_[j]) && (j >= 17))
                        j = 17;

                    z1 = z2;
                    n1 = n2;
                } // while
            } // main loop 

            nTot += n;
            
        } // ear loop

        if (input.getNEars() > 1)
            nTot /= input.getNEars();

        // Post-processing
        if (nTot < 0.0)
            nTot = 0.0;
        else if (nTot <= 16.0) // round to three dp
            nTot = std::floor (nTot * 1000.0 + 0.5) / 1000.0;
        else // round to two dp
            nTot = std::floor (nTot * 100.0 + 0.5) / 100.0;
        output_.setSample (0, 0, 0, nTot);
    }

   void InstantaneousLoudnessDIN45631::resetInternal(){};
}
