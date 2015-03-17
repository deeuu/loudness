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

#include "SignalBank.h"

namespace loudness{

    SignalBank::SignalBank()
    { 
        nEars_ = 0;
        nChannels_ = 0;
        nSamples_ = 0;
        fs_ = 0;
        trig_ = 0;
        initialized_ = false;
        frameRate_ =0;
    }

    SignalBank::~SignalBank() {}

    void SignalBank::initialize(int nEars, int nChannels, int nSamples, int fs)
    {
        if((nEars*nChannels*nSamples) == 0)
        {
            LOUDNESS_ERROR("SignalBank: Cannot generate signal(s) with this specification.");
            initialized_ = false;
        }
        else
        {
            nEars_ = nEars;
            nChannels_ = nChannels;
            nSamples_ = nSamples;
            nTotalSamples_ = nEars_ * nChannels_ * nSamples_;
            fs_ = fs;
            frameRate_ = fs_/(Real)nSamples_;
            trig_ = 1;
            initialized_ = true;

            centreFreqs_.assign(nChannels_, 0.0);
            signals_.assign(nTotalSamples_, 0.0);
        }
    }

    void SignalBank::initialize(const SignalBank &input)
    {
        if(input.isInitialized())
        {
            nEars_ = input.getNEars();
            nChannels_ = input.getNChannels();
            nSamples_ = input.getNSamples();
            nTotalSamples_ = input.getNTotalSamples();
            fs_ = input.getFs();
            frameRate_ = input.getFrameRate();
            trig_ = input.getTrig();
            initialized_ = true;
            centreFreqs_ = input.getCentreFreqs();
            signals_.assign(input.getNTotalSamples(), 0.0);
        }
        else
        {
            LOUDNESS_ERROR("SignalBank: Input SignalBank not initialized!");
            initialized_ = false;
        }
    }

    void SignalBank::clear()
    {
        signals_.assign(nTotalSamples_, 0.0);
    }

    void SignalBank::setFs(int fs)
    {
        fs_ = fs;
    }

    void SignalBank::setFrameRate(Real frameRate)
    {
        frameRate_ = frameRate;
    }

    void SignalBank::setCentreFreqs(const RealVec &centreFreqs)
    {
        centreFreqs_ = centreFreqs;
    }

    void SignalBank::setCentreFreq(int channel, Real freq)
    {
        if (channel<nChannels_)
           centreFreqs_[channel] = freq;
    }

    void SignalBank::fillSignal(int ear, int channel, int writeSampleIndex, const RealVec& source, int readSampleIndex, int nSamples)
    {
        int startIdx = (ear * nChannels_ * nSamples_ + channel * nSamples_);
        Real* write = &signals_[startIdx + writeSampleIndex];
        const Real* read = &source[readSampleIndex];
        for(int smp=0; smp<nSamples; smp++)
            *write++ = *read++;
    }

    void SignalBank::copySignalBank(const SignalBank& input)
    {
        if( input.getNTotalSamples() != nTotalSamples_)
        {
            LOUDNESS_WARNING("SignalBank: Total number of samples do not match");
        }
        else
        {
            signals_ = input.getSignals();
        }
    }

    void SignalBank::fillSignalBank(int writeSampleIndex, const SignalBank& input, int readSampleIndex, int nSamples)
    {
        Real* write = &signals_[writeSampleIndex];
        const Real *read = input.getSignalReadPointer(0, 0, readSampleIndex);
        int writeHop = nSamples_ - nSamples;
        int readHop = input.getNSamples() - nSamples;
        for(int i=0; i<(nEars_*nChannels_); i++)
        {
            for(int smp=0; smp<nSamples; smp++)
                *write++ = *read++;
            write += writeHop;
            read += readHop;
        }
    }

    void SignalBank::pullBack(int nSamples)
    {
        if (nSamples < nSamples_)
        {
            RealIter writeIter = signals_.begin();
            int remainingSamples_ = nSamples_ - nSamples;
            for(int i=0; i<(nEars_*nChannels_); i++)
            {
                RealIter readIter = writeIter + nSamples;
                RealIter endIter = readIter + remainingSamples_;
                for(int smp=0; smp<nSamples_; smp++)
                {
                    if(readIter < endIter)
                        *writeIter++ = *readIter++;
                    else
                        *writeIter++ = 0.0;
                }
            }
        }
        else
        {
            signals_.assign(nTotalSamples_, 0.0);
        }
    }
    
    const RealVec &SignalBank::getCentreFreqs() const
    {
        return centreFreqs_;
    }

    int SignalBank::getFs() const
    {
        return fs_;
    }

    Real SignalBank::getFrameRate() const
    {
        return frameRate_;
    }

    bool SignalBank::isInitialized() const
    {
        return initialized_;
    }
}
