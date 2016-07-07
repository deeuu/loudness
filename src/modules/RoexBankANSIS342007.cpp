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

#include "RoexBankANSIS342007.h"
#include "../support/AuditoryTools.h"

namespace loudness{

    RoexBankANSIS342007::RoexBankANSIS342007(Real camLo, Real camHi, Real camStep) : 
        Module("RoexBankANSIS342007"),
        camLo_(camLo), 
        camHi_(camHi),
        camStep_(camStep)
    {}

    RoexBankANSIS342007::~RoexBankANSIS342007()
    {
    }

    bool RoexBankANSIS342007::initializeInternal(const SignalBank &input)
    {
        //number of input components
        int nChannels = input.getNChannels();

        //number of roex filters to use
        nFilters_ = std::floor((camHi_ - camLo_) / camStep_) + 1;

        LOUDNESS_DEBUG(name_
                << " filter spacing in Cams: " << camStep_
                << " Total number of filters: " << nFilters_);

        //initialize output SignalBank
        output_.initialize (input.getNSources(),
                            input.getNEars(),
                            nFilters_,
                            1,
                            input.getFs());
        output_.setFrameRate (input.getFrameRate());
        output_.setChannelSpacingInCams (camStep_);

        //see ANSI S3.4 2007 p.11
        const Real p51_1k = 4000.0 / centreFreqToCambridgeERB (1000.0);

        //pcomp is slope per component
        pcomp_.assign (nChannels, 0.0);

        //comp_level holds level per ERB on each component
        compLevel_.assign (nChannels, 0.0);

        //p upper is level invariant
        pu_.assign (nFilters_, 0.0);
        
        //p lower is level dependent
        pl_.assign (nFilters_, 0.0);

        //fill the above arrays
        Real cam = 0.0, erb = 0.0, fc = 0.0;
        for (int i = 0; i < nChannels; ++i)
        {
            fc = input.getCentreFreq(i);
            erb = centreFreqToCambridgeERB (fc);
            pcomp_[i] = 4.0 * fc / erb;
        }
        for (int i = 0; i < nFilters_; ++i)
        {
            //filter frequency in Cams
            cam = camLo_ + (i * camStep_);
            //filter frequency in Hz
            fc = camToHertz (cam);
            //get the ERB of the filter
            erb = centreFreqToCambridgeERB (fc);
            //ANSI S3.4 sec 3.5 p.11
            pu_[i] = 4.0 * fc / erb;
            pl_[i] = 0.35 * (pu_[i] / p51_1k);
            output_.setCentreFreq (i, fc);
        }

        return 1;
    }


    void RoexBankANSIS342007::processInternal(const SignalBank &input)
    {
        for (int src = 0; src < input.getNSources(); ++src)
        {
            for (int ear = 0; ear < input.getNEars(); ++ear)
            {
                Real excitationLin = 0.0, fc = 0.0, g = 0.0, p = 0.0, pg = 0.0;
                int nChannels = input.getNChannels();
                const Real* inputPowerSpectrum = input
                                                 .getSingleSampleReadPointer
                                                 (src, ear, 0);
                Real* outputExcitationPattern = output_.
                                                getSingleSampleWritePointer
                                                (src, ear, 0);

                //ANSI 2007 style: calculate level per ERB
                //using level independent roex filters centred on every component
                int j = 0;
                for (int i = 0; i < nChannels; ++i)
                {
                    excitationLin = 0.0;
                    j = 0;
                    fc = input.getCentreFreq(i);

                    while (j < nChannels)
                    {
                        //normalised deviation
                        g = (input.getCentreFreq(j) - fc) / fc;

                        if (g > 2)
                            break;
                        if (g < 0) //lower value 
                            pg = -pcomp_[i] * g; //p*abs(g)
                        else //upper value
                            pg = pcomp_[i] * g;
                        //excitation per erb
                        excitationLin += (1 + pg) * exp (-pg) * inputPowerSpectrum[j++];
                    }

                    //convert to dB, subtract 51 here to save operations later
                    compLevel_[i] = powerToDecibels (excitationLin, 1e-10, -100.0) - 51;
                }
                
                //now the excitation pattern
                for (int i = 0; i < nFilters_; ++i)
                {
                    excitationLin = 0.0;
                    j = 0;
                    fc = output_.getCentreFreq(i);

                    while (j < nChannels)
                    {
                        //normalised deviation
                        g = (input.getCentreFreq(j) - fc) / fc;

                        if (g > 2)
                            break;
                        if (g < 0) //lower value 
                        {
                            //checked out 2.4.14
                            p = pu_[i] - (pl_[i] * compLevel_[j]); //51dB subtracted above
                            p = max(0.1, p); //p can go negative for very high levels
                            pg = -p * g; //p*abs(g)
                        }
                        else //upper value
                        {
                            pg = pu_[i] * g;
                        }

                        excitationLin += (1 + pg) * exp(-pg) * inputPowerSpectrum[j++];
                    }

                    outputExcitationPattern[i] = excitationLin;
                }
            }
        }
    }

    void RoexBankANSIS342007::resetInternal(){};
}
