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

    SignalBank::SignalBank() :
        nSources_(0),
        nEars_(0),
        nChannels_(0),
        nSamples_(0),
        trig_(false),
        initialized_(false),
        fs_(0),
        frameRate_(0),
        channelSpacingInCams_(0),
        reserveSamples_(0)
    {}

    SignalBank::~SignalBank() {}

    void SignalBank::initialize(int nSources, int nEars, int nChannels, int nSamples, int fs)
    {
        if((nEars*nChannels*nSamples*nSources) == 0)
        {
            LOUDNESS_ERROR("SignalBank: Cannot generate signal(s) with this specification.");
            initialized_ = false;
        }
        else
        {
            nSources_ = nSources;
            nEars_ = nEars;
            nChannels_ = nChannels;
            nSamples_ = nSamples;
            nTotalSamplesPerEar_ = nChannels_ * nSamples_;
            nTotalSamplesPerSource_ = nEars_ * nTotalSamplesPerEar_;
            nTotalSamples_ = nTotalSamplesPerSource_ * nSources_;
            fs_ = fs;
            frameRate_ = fs_;
            trig_ = 1;
            initialized_ = true;

            centreFreqs_.assign(nChannels_, 0.0);
            signals_.assign(nTotalSamples_, 0.0);
            aggregatedSignals_.clear();
            reserveSamples_ = nTotalSamples_ * 1000;

            LOUDNESS_DEBUG("SignalBank: Initialised.");
        }
    }

    void SignalBank::initialize(const SignalBank &input)
    {
        if(input.isInitialized())
        {
            nSources_ = input.getNSources();
            nEars_ = input.getNEars();
            nChannels_ = input.getNChannels();
            nSamples_ = input.getNSamples();
            nTotalSamples_ = input.getNTotalSamples();
            nTotalSamplesPerEar_ = input.getNTotalSamplesPerEar();
            nTotalSamplesPerSource_ = input.getNTotalSamplesPerSource();
            fs_ = input.getFs();
            frameRate_ = input.getFrameRate();
            trig_ = input.getTrig();
            initialized_ = true;
            centreFreqs_ = input.getCentreFreqs();
            channelSpacingInCams_ = input.getChannelSpacingInCams();
            signals_.assign(input.getNTotalSamples(), 0.0);
            aggregatedSignals_.clear();
            reserveSamples_ = nTotalSamples_ * 1000;
        }
        else
        {
            LOUDNESS_ERROR("SignalBank: Input SignalBank not initialized!");
            initialized_ = false;
        }
    }

    void SignalBank::reset()
    {
        signals_.assign(nTotalSamples_, 0.0);
        aggregatedSignals_.clear();
        trig_ = true;
    }

    bool SignalBank::hasSameShape(const SignalBank& input) const
    {
        if ( (input.getNSources() == nSources_)
                && (input.getNEars() == nEars_)
                && (input.getNChannels() == nChannels_)
                && (input.getNSamples() == nSamples_))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    void SignalBank::scale(Real gainFactor)
    {
        for (int i = 0; i < nTotalSamples_; ++i)
            signals_[i] *= gainFactor;
    }

    void SignalBank::scale(int source, int ear, Real gainFactor)
    {
        int start = source * nTotalSamplesPerSource_ + ear * nTotalSamplesPerEar_;
        int end = start + nTotalSamplesPerEar_;
        for (int i = start; i < end; ++i)
            signals_[i] *= gainFactor;
    }

    void SignalBank::scale(int ear, Real gainFactor)
    {
        int start = ear * nTotalSamplesPerEar_;
        int end = start + nTotalSamplesPerEar_;
        for (int i = start; i < end; ++i)
            signals_[i] *= gainFactor;
    }

    void SignalBank::scale(int source, int ear, int channel, Real gainFactor)
    {
        int start = ear * nTotalSamplesPerEar_ + channel * nSamples_;
        int end = start + nSamples_;
        for (int i = start; i < end; ++i)
            signals_[i] *= gainFactor;
    }

    void SignalBank::zeroSignals()
    {
        signals_.assign(nTotalSamples_, 0.0);
    }

    void SignalBank::clearAggregatedSignals()
    {
        aggregatedSignals_.clear();
    }

    void SignalBank::setFs(int fs)
    {
        fs_ = fs;
    }

    void SignalBank::setFrameRate(Real frameRate)
    {
        frameRate_ = frameRate;
    }

    void SignalBank::setChannelSpacingInCams(Real channelSpacingInCams)
    {
        channelSpacingInCams_ = channelSpacingInCams;
    }

    const Real SignalBank::getChannelSpacingInCams() const
    {
        return channelSpacingInCams_;
    }

    void SignalBank::setCentreFreqs(const RealVec &centreFreqs)
    {
        centreFreqs_ = centreFreqs;
    }

    void SignalBank::copySamples(
            int source, 
            int ear,
            int channel,
            int writeSampleIndex,
            const Real* input,
            int nSamples)
    {
        LOUDNESS_ASSERT(((nSamples+writeSampleIndex) <= nSamples_) &&
                isPositiveAndLessThanUpper(source, nSources_) &&
                isPositiveAndLessThanUpper(ear, nEars_) &&
                isPositiveAndLessThanUpper(channel, nChannels_));

        int startIdx = (source * nTotalSamplesPerSource_ +
                        ear * nTotalSamplesPerEar_ +
                        channel * nSamples_
                        + writeSampleIndex);
        Real* write = &signals_[startIdx];
        for (int smp = 0; smp < nSamples; ++smp)
            *write++ = *input++;
    }

    void SignalBank::copySamples(
            int source, 
            int ear,
            int channel,
            int writeSampleIndex,
            const float* input,
            int nSamples)
    {
        LOUDNESS_ASSERT(((nSamples+writeSampleIndex) <= nSamples_) &&
                isPositiveAndLessThanUpper(source, nSources_) &&
                isPositiveAndLessThanUpper(ear, nEars_) &&
                isPositiveAndLessThanUpper(channel, nChannels_));

        int startIdx = (source * nTotalSamplesPerSource_ +
                        ear * nTotalSamplesPerEar_ +
                        channel * nSamples_
                        + writeSampleIndex);
        Real* write = &signals_[startIdx];
        for (int smp = 0; smp < nSamples; ++smp)
            *write++ = *input++;
    }

    void SignalBank::copySamples(const SignalBank& input)
    {
        LOUDNESS_ASSERT(hasSameShape(input), "SignalBank: Dimensions do not match");
        signals_ = input.getSignals();
    }

    void SignalBank::copySamples(
            int writeSampleIndex,
            const SignalBank& input,
            int readSampleIndex, 
            int nSamples)
    {
        LOUDNESS_ASSERT(((nSamples + writeSampleIndex) <= nSamples_)
                        && ((nSamples + readSampleIndex)
                             <= input.getNSamples())
                        && (input.getNSources() == nSources_)
                        && (input.getNEars() == nEars_)
                        && (input.getNChannels() == nChannels_));

        Real* write = &signals_[writeSampleIndex];
        const Real *read = input.getSignalReadPointer(0, 0, 0, readSampleIndex);
        int writeHop = nSamples_ - nSamples;
        int readHop = input.getNSamples() - nSamples;

        for (int i = 0; i < (nSources_ * nEars_ * nChannels_); ++i)
        {
            for(int smp=0; smp<nSamples; smp++)
                *write++ = *read++;
            write += writeHop;
            read += readHop;
        }
    }

    void SignalBank::aggregate()
    {  
        aggregatedSignals_.reserve(reserveSamples_);
        aggregatedSignals_.insert (aggregatedSignals_.end(), signals_.begin(), signals_.end());
    }

    void SignalBank::pullBack(int nSamples)
    {
        if (nSamples < nSamples_)
        {
            RealIter writeIter = signals_.begin();
            int remainingSamples_ = nSamples_ - nSamples;
            for (int i = 0; i < (nSources_ * nEars_ * nChannels_); ++i)
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
   
    const RealVec& SignalBank::getCentreFreqs() const
    {
        return centreFreqs_;
    }

    const Real* SignalBank::getCentreFreqsReadPointer(int channel) const
    {
        LOUDNESS_ASSERT(isPositiveAndLessThanUpper(channel, nChannels_));
        return &centreFreqs_[channel];
    }

    Real* SignalBank::getCentreFreqsWritePointer(int channel)
    {
        LOUDNESS_ASSERT(isPositiveAndLessThanUpper(channel, nChannels_));
        return &centreFreqs_[channel];
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
