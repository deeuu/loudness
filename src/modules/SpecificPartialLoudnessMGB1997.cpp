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

#include "SpecificPartialLoudnessMGB1997.h"

namespace loudness{

    SpecificPartialLoudnessMGB1997::SpecificPartialLoudnessMGB1997(
            bool useANSISpecificLoudness,
            bool updateParameterCForBinauralInhibition) :
        Module("SpecificPartialLoudnessMGB1997"),
        useANSISpecificLoudness_(useANSISpecificLoudness),
        updateParameterCForBinauralInhibition_(updateParameterCForBinauralInhibition)
    {}

    SpecificPartialLoudnessMGB1997::~SpecificPartialLoudnessMGB1997() {}

    Real SpecificPartialLoudnessMGB1997::internalExcitation(Real freq)
    {
        if (freq>=500)
        {
            return 3.73;
        }
        else
        {
            Real logFreq;
            if(freq<=50)
                logFreq = log10(50);
            else
                logFreq = log10(freq);

            return pow(10, (0.43068810954936998 * pow(logFreq,3) 
                        - 2.7976098820730675 * pow(logFreq,2) 
                        + 5.0738460335696969 * logFreq -
                        1.2060617476790148));
        }
    };

    Real SpecificPartialLoudnessMGB1997::gdBToA(Real gdB)
    {
        if(gdB>=0)
        {
            return 4.72096;
        }
        else
        {
            return -0.0000010706497192096045 * pow(gdB,5) 
                -0.000060648487122230512 * pow(gdB,4) 
                -0.0012047326575717733 * pow(gdB,3) 
                -0.0068190417911848525 * pow(gdB,2)
                -0.11847825641628305 * gdB
                + 4.7138722463497347;
        }
    }

    Real SpecificPartialLoudnessMGB1997::gdBToAlpha(Real gdB)
    {
        if(gdB>=0)
        {
            return 0.2;
        }
        else
        {
            return 0.000026864285714285498 * pow(gdB,2)
                -0.0020023357142857231 * gdB + 0.19993107142857139;
        }
    }

    Real SpecificPartialLoudnessMGB1997::kdB(Real freq)
    {
        if (freq>=1000)
        {
            return -3.0;
        }
        else
        {
            Real logFreq;
            if(freq<=50)
                logFreq = log10(50);
            else
                logFreq = log10(freq);

            return 4.4683421395470813 * pow(logFreq,4) 
                - 49.680265975480935 * pow(logFreq,3) 
                + 212.89091871289099 * pow(logFreq,2) 
                - 418.88552055717832 * logFreq + 317.07466358701885;
        }
    };

    void SpecificPartialLoudnessMGB1997::setParameterC(Real parameterC)
    {
        parameterC_ = parameterC;
    }

    bool SpecificPartialLoudnessMGB1997::initializeInternal(const SignalBank &input)
    {
        //c value from ANSI 2007
        parameterC_ = 0.046871;

        if (useANSISpecificLoudness_)
        {
            yearExp_ = 0.2;
            parameterC2_ = parameterC_ / std::pow(1.0707, 0.5);
        }
        else
        {
            yearExp_ = 0.5;
            parameterC2_ = parameterC_ / std::pow(1040000.0, 0.5);
        }

        if (updateParameterCForBinauralInhibition_)
        {
            parameterC_ /= 0.75;
            LOUDNESS_DEBUG(name_ 
                    << ": Scaling parameter C for binaural inhibition model: "
                    << parameterC_);
        }

        Real eThrqdB500Hz = internalExcitation(500);
        //fill loudness parameter vectors
        for (int i = 0; i < input.getNChannels(); i++)
        {
            Real fc = input.getCentreFreq (i);
            Real eThrqdB = internalExcitation (fc);
            Real gdB = eThrqdB500Hz - eThrqdB;
            eThrqParam_.push_back (std::pow (10, eThrqdB / 10.0));
            gParam_.push_back (std::pow (10, gdB / 10.0));
            aParam_.push_back (gdBToA (gdB));
            alphaParam_.push_back (gdBToAlpha (gdB));
            kParam_.push_back (std::pow (10, kdB (fc) / 10.0));
        }

        //output SignalBank
        output_.initialize (input);

        return 1;
    }

    void SpecificPartialLoudnessMGB1997::processInternal(const SignalBank &input)
    {
        for (int ear = 0; ear < input.getNEars(); ++ear)
        {
            // excitations were calculated using same roex shapes
            // so linear superposition of excitations applies
            RealVec eTot (input.getNChannels(), 0.0);
            for (int src = 0; src < input.getNSources(); ++src)
            {
                const Real* currentSignal = input
                                            .getSingleSampleReadPointer
                                            (src, ear, 0);

                for (int chn = 0; chn < input.getNChannels(); ++chn)
                    eTot[chn] += currentSignal[chn];
            }

            // Loudness for each source in the presence of all other sources
            for (int src = 0; src < input.getNSources(); ++src)
            {
                const Real* eSig = input
                                   .getSingleSampleReadPointer
                                   (src, ear, 0);

                Real* specLoud = output_
                                 .getSingleSampleWritePointer
                                 (src, ear, 0);

                for (int chn = 0; chn < input.getNChannels(); ++chn)
                {
                    Real eNoise = eTot[chn] - eSig[chn];
                    Real eThrn = kParam_[chn] * eNoise + eThrqParam_[chn];
                    Real nSig = 0.0;

                    if (eSig[chn] > 1e-10)
                    {
                        if (eTot[chn] > 1e10)
                        {
                            if (eSig[chn] >= eThrn) // Equation 19
                            {
                                nSig = parameterC2_ *
                                       std::pow (eTot[chn], yearExp_);

                                nSig -= parameterC2_ * 
                                        (std::pow (eNoise + eThrn, yearExp_) -
                                        std::pow (eThrqParam_[chn] *
                                        gParam_[chn] + aParam_[chn],
                                        alphaParam_[chn]) +
                                        std::pow (aParam_[chn],
                                        alphaParam_[chn])) *
                                        std::pow (eThrn / eSig[chn], 0.3);
                            }
                            else // Equation 20
                            {
                                nSig = parameterC_ * std::pow (2.0 * eSig[chn]
                                       / (eSig[chn] + eThrn), 1.5);

                                nSig *= (std::pow (eThrqParam_[chn] *
                                        gParam_[chn] + aParam_[chn],
                                        alphaParam_[chn]) - 
                                        std::pow (aParam_[chn],
                                        alphaParam_[chn])) /
                                        (std::pow (eNoise + eThrn, yearExp_) -
                                        std::pow(eNoise, yearExp_));
                                nSig *= std::pow (eTot[chn], yearExp_)
                                        - std::pow (eNoise, yearExp_);
                            }
                        }
                        else // eTot <= 100 dB
                        {
                            if (eSig[chn] >= eThrn) // Equation 17
                            {
                                nSig = parameterC_ * (std::pow (eTot[chn]
                                       * gParam_[chn] + aParam_[chn],
                                       alphaParam_[chn]) - std::pow (
                                       aParam_[chn], alphaParam_[chn]));

                                nSig -= parameterC_ * (std::pow (
                                       gParam_[chn] * (eNoise + eThrn) +
                                       aParam_[chn], alphaParam_[chn]) -
                                       std::pow (eThrqParam_[chn] * gParam_[chn]
                                       + aParam_[chn], alphaParam_[chn]))
                                       * std::pow (eThrn / eSig[chn], 0.3);
                            }
                            else // Equation 18
                            {
                                nSig = parameterC_ * std::pow (2.0 * eSig[chn]
                                       / (eSig[chn] + eThrn), 1.5);
                                
                                nSig *= (std::pow (eThrqParam_[chn] *
                                        gParam_[chn] + aParam_[chn],
                                        alphaParam_[chn]) - std::pow (
                                        aParam_[chn], alphaParam_[chn])) /
                                        (std::pow ((eNoise + eThrn) *
                                        gParam_[chn] + aParam_[chn],
                                        alphaParam_[chn]) - std::pow (
                                        eNoise * gParam_[chn] + aParam_[chn],
                                        alphaParam_[chn]));

                                nSig *= std::pow (eTot[chn] * gParam_[chn] +
                                        aParam_[chn], alphaParam_[chn]) -
                                        std::pow (eNoise * gParam_[chn] +
                                        aParam_[chn], alphaParam_[chn]);
                            }
                        }
                    }

                    specLoud[chn] = nSig;
                }
            }
        }
    }

    void SpecificPartialLoudnessMGB1997::resetInternal(){};
}
