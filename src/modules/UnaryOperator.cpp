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

#include "UnaryOperator.h"

namespace loudness{

    UnaryOperator::UnaryOperator(const OP &op, Real scale, Real offset) :
        Module("UnaryOperator"),
        op_ (op),
        scale_ (scale),
        offset_ (offset)
    {
        LOUDNESS_DEBUG(name_ << ": Constructed.");
    }

    UnaryOperator::~UnaryOperator()
    {};

    bool UnaryOperator::initializeInternal(const SignalBank &input)
    {
        int nOutputSamples = 0;
        switch (op_)
        {
            case LOG:
                nOutputSamples = input.getNSamples();
            case LOG10:
                nOutputSamples = input.getNSamples();
        }

        //output SignalBank
        output_.initialize (input.getNSources(),
                            input.getNEars(),
                            input.getNChannels(),
                            nOutputSamples,
                            input.getFs());

        output_.setFrameRate (input.getFrameRate());
        output_.setCentreFreqs (input.getCentreFreqs());

        return 1;
    }

    void UnaryOperator::processInternal(const SignalBank &input)
    {       
        for (int src = 0; src < input.getNSources(); ++src)
        {
            for (int ear = 0; ear < input.getNEars(); ++ear)
            {
                for (int chn = 0; chn < input.getNChannels(); ++chn)
                {
                    const Real* x = input.getSignalReadPointer(ear, chn, 0);
                    Real* y = output_.getSignalWritePointer(ear, chn, 0);

                    switch (op_)
                    {
                        case LOG:
                        {
                            for (int smp = 0; smp < input.getNSamples(); ++smp)
                            {
                                if (x[smp] > 1e-10)
                                    y[smp] = std::log (x[smp]);
                                else
                                    y[smp] = std::log (1e-10);
                            }
                        }
                        case LOG10:
                        {
                            for (int smp = 0; smp < input.getNSamples(); ++smp)
                            {
                                if (x[smp] > 1e-10)
                                    y[smp] = std::log10 (x[smp]);
                                else
                                    y[smp] = std::log10 (1e-10);
                            }
                        }
                    }

                }
            }
        }

        Real* y = output_.getSignalWritePointer(0, 0, 0);
        if (scale_ != 1.0)
        {
            for (int smp = 0; smp < output_.getNTotalSamples(); ++smp)
                y[smp] *= scale_;
        }
        if (offset_ != 0.0)
        {
            for (int smp = 0; smp < output_.getNTotalSamples(); ++smp)
                y[smp] += offset_;
        }
    }

    //output SignalBanks are cleared so not to worry about filter state
    void UnaryOperator::resetInternal()
    {}
}
