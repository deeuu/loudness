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

#include "ComplexGammatoneBank.h"

namespace loudness{

    ComplexGammatoneBank::ComplexGammatoneBank(
            Real camLo,
            Real camHi,
            Real camStep,
            int order,
            bool outputEnvelope) :
        Module("ComplexGammatoneBank"),
        camLo_ (camLo),
        camHi_ (camHi),
        camStep_ (camStep),
        order_ (order),
        outputEnvelope_ (outputEnvelope)
    {}

    ComplexGammatoneBank::~ComplexGammatoneBank() {}

    bool ComplexGammatoneBank::initializeInternal(const SignalBank &input)
    {
        int nFilters = std::floor((camHi_ - camLo_) / camStep_) + 1;
        realCoef_.assign (nFilters, 0.0);
        imagCoef_.assign (nFilters, 0.0);
        normFactor_.assign (nFilters, 0.0);

        //internal delay line - single vector for all
        realPrev_.initialize(input.getNSources(),
                             input.getNEars(),
                             nFilters,
                             order_ * 2,
                             1);

        output_.initialize (input.getNSources(),
                            input.getNEars(),
                            nFilters,
                            input.getNSamples(),
                            input.getFs());

        int fs = input.getFs();
        for (int chn = 0; chn < nFilters; ++chn)
        {
            Real fc = camToHertz (camLo_ + chn * camStep_);
            Real erb = centreFreqToCambridgeERB (fc);
            /*
            Real l = 24.7;
            Real q = 9.265;
            Real fc = (std::exp ((camLo_ + chn * camStep_) / q) -
                      1.0) * l * q;
            Real erb = l + fc / q;
            */
            output_.setCentreFreq (chn, fc);

            Real aNum = PI * factorial (2.0 * order_ - 2.0) * 
                        std::pow (2.0, -(2.0 * order_ - 2.0));
            Real aDen = std::pow (factorial (order_ - 1.0), 2.0);
            Real a = aNum / aDen;

            Real c = 2.0 * std::sqrt (std::pow (2.0, 1.0 / order_) - 1.0);

            Real bw3dB = erb * c / a;

            Real beta = 2 * PI * fc / fs;

            Real phi = PI * bw3dB / fs;

            Real u = -3.0 / 4.0;

            Real pNum = -2.0 + 2.0 * std::pow (10.0, u / 10.0) *
                        std::cos (phi);
            Real pDen = 1 - std::pow (10, u / 10.0);
            Real p = pNum / pDen;

	    Real lambda = -p * 0.5 - std::sqrt (p * p * 0.25 - 1.0);

	    realCoef_[chn] = lambda * std::cos (beta);
	    imagCoef_[chn] = lambda * std::sin (beta);
	    normFactor_[chn] = 2.0 * std::pow (1.0 - abs (lambda), 4);

            LOUDNESS_DEBUG(name_ << " FC: " << fc << ", ERB: " << erb);
            LOUDNESS_DEBUG(name_ << " normFactor: " << normFactor_[chn]);
            LOUDNESS_DEBUG(name_ << " Real coef: " << realCoef_[chn]);
            LOUDNESS_DEBUG(name_ << " Imag coef: " << imagCoef_[chn]);
        }

        return 1;
    }

    void ComplexGammatoneBank::processInternal(const SignalBank &input)
    {

        for (int src = 0; src < input.getNSources(); ++src)
        {
            for (int ear = 0; ear < input.getNEars(); ++ear)
            {
                const Real* inputSignal = input
                                          .getSignalReadPointer
                                          (src, ear, 0);
                for (int chn = 0; chn < output_.getNChannels(); ++chn)
                {
                    Real* outputSignal = output_
                                         .getSignalWritePointer(src, ear, chn);
                    
                    Real* realPrev = realPrev_
                                     .getSignalWritePointer(src, ear, chn);

                    Real* imagPrev = imagPrev_
                                     .getSignalWritePointer(src, ear, chn);
                                              
                    for (int smp = 0; smp < input.getNSamples(); ++smp)
                    {
                        Real real = inputSignal[smp];
                        Real imag = 0.0;

                        for (int i = 0; i < order_; ++i)
                        {
                            real += realCoef_[chn] * realPrev[i] -
                                    imagCoef_[chn] * imagPrev[i];

                            imag += realCoef_[chn] * imagPrev[i] +
                                    imagCoef_[chn] * realPrev[i];

                            realPrev[i] = real;
                            imagPrev[i] = imag;
                        }

                        if (outputEnvelope_)
                        {
                            outputSignal[smp] = sqrt
                                                (real * real + imag * imag);
                            outputSignal[smp] *= normFactor_[chn];
                        }
                        else
                        {
                            outputSignal[smp] = real * normFactor_[chn];
                        }
                    }
                }
            }
        }
    }

    void ComplexGammatoneBank::resetInternal()
    {
        realPrev_.zeroSignals();
        imagPrev_.zeroSignals();
    }
}
