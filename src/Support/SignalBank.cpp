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
        nChannels_ = 0;
        nSamples_ = 0;
        fs_ = 0;
        trig_ = 0;
        initialized_ = false;
        frameRate_ =0;
    }

    SignalBank::~SignalBank()
    {
    }

    void SignalBank::initialize(int nChannels, int nSamples, int fs)
    {
        nChannels_ = nChannels;
        nSamples_ = nSamples;
        fs_ = fs;
        frameRate_ = fs_/(Real)nSamples_;
        trig_ = 1;
        initialized_ = true;
        centreFreqs_.assign(nChannels_, 0.0);

        signal_.resize(nChannels_);
        for(int i=0; i<nChannels_; i++)
            signal_[i].assign(nSamples_,0.0);
    }

    void SignalBank::initialize(const SignalBank &input)
    {
        if(input.isInitialized())
        {
            nChannels_ = input.getNChannels();
            nSamples_ = input.getNSamples();
            fs_ = input.getFs();
            frameRate_ = input.getFrameRate();
            trig_ = input.getTrig();
            initialized_ = true;
            centreFreqs_ = input.getCentreFreqs();
            signal_.resize(nChannels_);
            for(int i=0; i<nChannels_; i++)
                signal_[i].assign(nSamples_,0.0);
        }
        else
        {
            LOUDNESS_ERROR("SignalBank: Input SignalBank not initialized!");
            initialized_ = false;
        }
    }

    void SignalBank::clear()
    {
        for(int i=0; i<nChannels_; i++)
            signal_[i].assign(nSamples_,0.0);
        trig_ = 1;
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

    void SignalBank::setSignal(int channel, const RealVec &signal)
    {
        if(channel<nChannels_ && (int)signal.size()==nSamples_)
            signal_[channel] = signal;
        else
        {
            LOUDNESS_ERROR("SignalBank: "
                    << "Invalid channel index or signal lengths do not match, please correct.");
        }
    }
    
    void SignalBank::fillSignal(int channel, int writeSampleIndex, const RealVec& source, int readSampleIndex, int nSamples)
    {
        for(int i=0; i<nSamples; i++)
            signal_[channel][writeSampleIndex++] = source[readSampleIndex++];
    }

    void SignalBank::pullSignalBack(int channel, int nSamples)
    {
        for(int i=0; i<nSamples_; i++)
        {
            if(nSamples<nSamples_)
                signal_[channel][i] = signal_[channel][nSamples++];
            else
                signal_[channel][i] = 0.0;
        }
    }

    const RealVec &SignalBank::getSignal(int channel) const
    {
        return signal_[channel];
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

