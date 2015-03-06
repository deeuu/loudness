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

    SignalBank::~SignalBank()
    {
    }

    void SignalBank::initialize(int nChannels, int nSamples, int fs)
    {
        nEars_ = 1;
        nChannels_ = nChannels;
        nSamples_ = nSamples;
        fs_ = fs;
        frameRate_ = fs_/(Real)nSamples_;
        trig_ = 1;
        initialized_ = true;
        centreFreqs_.assign(nChannels_, 0.0);

        signal_.clear();
        signal_.push_back(RealVecVec(nChannels_, RealVec(nSamples, 0.0)));
    }

    void SignalBank::initialize(int nEars, int nChannels, int nSamples, int fs)
    {
        nEars_ = nEars;
        nChannels_ = nChannels;
        nSamples_ = nSamples;
        fs_ = fs;
        frameRate_ = fs_/(Real)nSamples_;
        trig_ = 1;
        initialized_ = true;
        centreFreqs_.assign(nChannels_, 0.0);

        signal_.clear();
        for(int i=0; i<nEars_; i++)
            signal_.push_back(RealVecVec(nChannels_, RealVec(nSamples, 0.0)));

    }

    void SignalBank::initialize(const SignalBank &input)
    {
        if(input.isInitialized())
        {
            nEars_ = input.getNEars();
            nChannels_ = input.getNChannels();
            nSamples_ = input.getNSamples();
            fs_ = input.getFs();
            frameRate_ = input.getFrameRate();
            trig_ = input.getTrig();
            initialized_ = true;
            centreFreqs_ = input.getCentreFreqs();
            signal_.clear();
            for(int i=0; i<nEars_; i++)
                signal_.push_back(RealVecVec(nChannels_, RealVec(nSamples_, 0)));
        }
        else
        {
            LOUDNESS_ERROR("SignalBank: Input SignalBank not initialized!");
            initialized_ = false;
        }
    }

    void SignalBank::resizeSignal(int channel, int nSamples)
    {
        if (channel < nChannels_)
            signal_[0][channel].assign(nSamples, 0.0);
    }

    void SignalBank::resizeSignal(int ear, int channel, int nSamples)
    {
        if(ear<nEars_)
        {
            if (channel < nChannels_)
                signal_[ear][channel].assign(nSamples, 0.0);
        }
    }

    void SignalBank::clear()
    {
        for(int i=0; i<nEars_; i++)
        {
            for(int j=0; j<nChannels_; j++)
                signal_[i][j].assign(nSamples_,0.0);
        }
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
            signal_[0][channel] = signal;
        else
        {
            LOUDNESS_ERROR("SignalBank: "
                    << "Invalid channel index or signal lengths do not match, please correct.");
        }
    }
    
    void SignalBank::fillSignal(int channel, int writeSampleIndex, const RealVec& source, int readSampleIndex, int nSamples)
    {
        for(int i=0; i<nSamples; i++)
            signal_[0][channel][writeSampleIndex++] = source[readSampleIndex++];
    }

    void SignalBank::fillSignal(int ear, int channel, int writeSampleIndex, const RealVec& source, int readSampleIndex, int nSamples)
    {
        for(int i=0; i<nSamples; i++)
            signal_[ear][channel][writeSampleIndex++] = source[readSampleIndex++];
    }

    void SignalBank::pullBack(int nSamples)
    {
        for(int ear=0; ear < nEars_; ear++)
        {
            for (int chn=0; chn < nChannels_; chn++)
            {
                int sample = nSamples;
                for (int i=0; i < nSamples_; i++)
                {
                    if(sample<nSamples_)
                        signal_[ear][chn][i] = signal_[ear][chn][sample++];
                    else
                        signal_[ear][chn][i] = 0.0;
                }
            }
        }
    }

    const RealVec &SignalBank::getSignal(int channel) const
    {
        return signal_[0][channel];
    }

    const RealVec &SignalBank::getSignal(int ear, int channel) const
    {
        return signal_[ear][channel];
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

