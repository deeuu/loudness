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

#include "MultiSourceRoexBank.h"
#include "../support/AuditoryTools.h"

namespace loudness{

    MultiSourceRoexBank::MultiSourceRoexBank(Real camStep) :
        Module("MultiSourceRoexBank"),
        camStep_(camStep)
    {}

    MultiSourceRoexBank::~MultiSourceRoexBank() {}

    bool MultiSourceRoexBank::initializeInternal(const SignalBank &input)
    {

        /*
         *@TODO check component freqs are ascending
         */

        /*
         * Level per ERB precalculations
         */
        rectBinIndices_.resize(input.getNChannels());

        for (int i = 0; i < input.getNChannels(); ++i)
        {
            //ERB number of Centre frequency
            Real cam = hertzToCam (input.getCentreFreq(i));

            //rectangular ERB band edges in Hz
            Real freqLo = camToHertz (cam - 0.5);
            Real freqHi = camToHertz (cam + 0.5);

            //lower and upper bin indices
            rectBinIndices_[i].resize(2);
            rectBinIndices_[i][0] = i;
            rectBinIndices_[i][1] = i + 1;

            /* Find components falling within the band:
             * This follows the Fortran code in Glasberg and Moore's 1990 paper
             * which uses the inclusive interval [fLo, fHi].  An alternative
             * approach is to find the nearest DFT bins to the lower and upper
             * band edges, which would require a nearest search (bins may not be
             * uniformly spaced). Decided to go with the first prodedure for
             * simplicity and follow original code.
             */
            bool first = true;
            int j = 0;
            while (j < input.getNChannels())
            {
                if (input.getCentreFreq(j) > freqHi) 
                    break;
                else if (input.getCentreFreq(j) >= freqLo)
                {
                    if(first)
                        rectBinIndices_[i][0] = j;
                    first = false;
                    rectBinIndices_[i][1] = j + 1;
                }
                j++;
            }
        }	

        /*
         * Excitation pattern variables
         */

        //number of roex filters to use
        Real camLo = 1.8;
        Real camHi = 38.9;
        nFilters_ = std::floor((camHi - camLo) / camStep_) + 1;

        //see ANSI S3.4 2007 p.11
        const Real p51_1k = 4000.0 / centreFreqToCambridgeERB (1000.0);

        //p upper is level invariant
        pu_.assign (nFilters_, 0.0);
        
        //p lower is level dependent
        pl_.assign (nFilters_, 0.0);

        output_.initialize (input.getNSources(),
                            input.getNEars(),
                            nFilters_,
                            1,
                            input.getFs());
        output_.setChannelSpacingInCams (camStep_);
        output_.setFrameRate (input.getFrameRate());

        //fill the above arrays and calculate roex filter response for upper skirt
        for (int i = 0; i < nFilters_; ++i)
        {
            //filter frequency in Cams
            Real cam = camLo + (i * camStep_);

            Real fc = camToHertz (cam);
            output_.setCentreFreq (i, fc);

            //get the ERB of the filter
            Real erb = centreFreqToCambridgeERB (fc);
            //ANSI S3.4 sec 3.5 p.11
            pu_[i] = 4.0 * fc / erb;
            //from Eq (3)
            pl_[i] = 0.35 * (pu_[i] / p51_1k);
        }
        
        //generate lookup table for rounded exponential
        generateRoexTable(1024);
        
        return 1;
    }

    void MultiSourceRoexBank::processInternal(const SignalBank &input)
    {
        for (int ear = 0; ear < input.getNEars(); ++ear)
        {
            /*
             * Level per ERB given all sources in this ear
             */
            RealVec compLevel (input.getNChannels(), 0.0);
            for (int src = 0; src < input.getNSources(); ++src)
            {
                const Real* inputPowerSpectrum = input
                                                 .getSingleSampleReadPointer
                                                 (src, ear, 0);
                Real runningSum = 0.0;
                int j = 0;
                int k = rectBinIndices_[0][0];
                for (int i = 0; i < input.getNChannels(); ++i)
                {
                    //running sum of component powers
                    while (j < rectBinIndices_[i][1])
                        runningSum += inputPowerSpectrum[j++];

                    //subtract components outside the window
                    while (k < rectBinIndices_[i][0])
                        runningSum -= inputPowerSpectrum[k++];
                    compLevel[i] += runningSum;
                }
            }

            //convert to dB, subtract 51 here to save operations later
            for (int i = 0; i < input.getNChannels(); ++i)
                compLevel[i] = powerToDecibels (compLevel[i], 1e-10, -100.0) - 51;

            // Calculate a filter based on all inputs, then excitation per
            // per band per source
            for (int i = 0; i < nFilters_; ++i)
            {
                // shape
                int j = 0;
                RealVec roex (input.getNChannels(), 0.0);
                while (j < input.getNChannels())
                {
                    //normalised deviation
                    Real fc = output_.getCentreFreq(i);
                    Real g = (input.getCentreFreq(j) - fc) / fc;
                    Real p = 0.0, pg = 0.0;

                    if (g > 2)
                        break;
                    if (g < 0) //lower skirt - level dependent
                    {
                        //Complete Eq (3)
                        p = pu_[i] - (pl_[i] * compLevel[j]); //51dB subtracted above
                        p = max(p, 0.1); //p can go negative for very high levels
                        pg = -p * g; //p * abs (g)
                    }
                    else //upper skirt
                    {
                        pg = pu_[i] * g; //p * abs(g)
                    }
                    
                    //excitation
                    int idx = (int)(pg / step_ + 0.5);
                    idx = min (idx, roexIdxLimit_);
                    roex[j++] = roexTable_[idx];
                }

                // filter excitation for each source
                for (int src = 0; src < input.getNSources(); ++src)
                {
                    const Real* inputPowerSpectrum = input
                                                     .getSingleSampleReadPointer
                                                     (src, ear, 0);
                    Real* outputExcitationPattern = output_
                                                    .getSingleSampleWritePointer
                                                    (src, ear, 0);
                    outputExcitationPattern[i] = 0.0;
                    for (int k = 0; k < j; ++k)
                        outputExcitationPattern[i] += roex[k] * inputPowerSpectrum[k];
                }
            }
        }
    }

    void MultiSourceRoexBank::resetInternal(){};

    void MultiSourceRoexBank::generateRoexTable(int size)
    {
        size = max (size, 512);
        roexIdxLimit_ = size - 1;
        roexTable_.resize (size);

        double pgLim = 20.48; //end value is 20.46
        double pg;
        step_ = pgLim / size;
        
        for (int i = 0; i < size; ++i)
        {
            pg = step_ * i;
            roexTable_[i] = (1 + pg) * exp (-pg);
        }
    }
}
