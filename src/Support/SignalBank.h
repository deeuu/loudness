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

#ifndef SIGNALBANK_H
#define SIGNALBANK_H

#include "Common.h"

namespace loudness{

    /**
     * @class SignalBank
     * 
     * @brief A bank of signals used as inputs and outputs of processing modules.
     *
     * A SignalBank consists of a number of channels each with the same number
     * of samples. SignalBank's carry metadata, such as the sampling frequency
     * and the channel centre frequencies, which is useful for sharing
     * information between processing modules. After calling the constructor, a
     * SignalBank must be initialised using the initialize() function.
     *
     * SignalBank triggers are used to instruct modules when to process the
     * input. A trigger must equal 1 (default) for a module to process the
     * SignalBank, otherwise the output is not updated. This is useful for
     * frequency domain processing modules where the spectral processing rate is
     * typically lower than the host rate.
     * 
     * @author Dominic Ward
     */
    class SignalBank
    {
    public:
        SignalBank();
        ~SignalBank();

        /**
         * @brief Initialises the SignalBank with input arguments.
         *
         * @param n_channels Number of channels.
         * @param n_samples Number of samples per channel.
         * @param fs Sampling frequency.
         */
        void initialize(int nChannels, int nSamples, int fs);

        /**
         * @brief Initialises the SignalBank with the same parameters as the 
         * input.
         *
         * @param input The input SignalBank whose structure will be copied.
         */
        void initialize(const SignalBank &input);

        /**
         * @brief Clears the contents of SignalBank and sets trig to 1.
         */
        void clear();

        /*
         * setters
         */

        /**
         * @brief Sets the sampling frequency of Signal.
         *
         * @param fs The sampling frequency in Hz.
         */
        void setFs(int fs);
 
        /**
         * @brief Sets the frame rate of the Signal.
         *
         * Default is fs_ / nSamples_
         *
         * @param frameRate The frame rate.
         */
        void setFrameRate(Real frameRate);

        /**
         * @brief Sets the centre frequencies of all channels.
         *
         * @param centreFreqs A vector of centre frequencies in Hz.
         */
        void setCentreFreqs(const RealVec &centreFreqs);

        /**
         * @brief Sets the centre frequency of a single channel.
         *
         * @param channel Channel index.
         * @param freq The centre frequency (Hz).
         */
        void setCentreFreq(int channel, Real freq);

        /**
         * @brief Sets the signal of a specific channel.
         *
         * Signal length must be nSamples_.
         *
         * @param channel Channel index.
         * @param signal Signal vector.
         */
        void setSignal(int channel, const RealVec &signal);

        /**
         * @brief Sets the value of an individual sample in a specified channel.
         *
         * @param channel Channel index.
         * @param index Sample index.
         * @param sample Value of the sample.
         */

        inline void setSample(int channel, int index, Real sample) 
        {
            signal_[channel][index] = sample;
        }

        void fillSignal(int channel, int writeSampleIndex, const
                RealVec& source, int readSampleIndex, int nSamples);

        void pullBack(int nSamples);

        /**
         * @brief Sets the trigger state of the SignalBank.
         *
         * This function is mainly for use with PowerSpectrum.
         *
         * @param true if active, false otherwise.
         */
        inline void setTrig(bool trig)
        {
            trig_ = trig;
        }

        /*
         * Getters
         */

        /**
         * @brief Returns the trigger of the SignalBank.
         *
         * @return true if active, false otherwise.
         */
        inline bool getTrig() const
        {
            return trig_;
        }

        /**
         * @brief Returns the number of channels in the SignalBank.
         *
         * @return Number of channels.
         */
        inline int getNChannels() const
        {
            return nChannels_;
        }

        /**
         * @brief Returns the number of samples per channel.
         *
         * @return Number of samples.
         */
        inline int getNSamples() const
        {
            return nSamples_;
        }

        /**
         * @brief Returns a single sample from a specified channel and sample index.
         *
         * @param channel Channel index.
         * @param index Sample index.
         *
         * @return Sample value.
         */
        inline Real getSample(int channel, int index) const
        {
            return signal_[channel][index];
        }

        const RealVec &getSignal(int channel) const;

        /**
         * @brief Returns the centre frequency of a channel.
         *
         * @param channel Channel index.
         *
         * @return The centre frequency (Hz).
         */
        inline Real getCentreFreq(int channel) const
        {
            return centreFreqs_[channel];
        }

        /**
         * @brief Returns a vector of centre frequencies corresponding to each
         * channel.
         *
         * @return Vector of centre frequencies in Hz.
         */
        const RealVec &getCentreFreqs() const;

        /**
         * @brief Returns the sampling frequency.
         *
         * @return The sampling frequency in Hz.
         */
        int getFs() const;

        /**
         * @brief Returns the frame rate of the SignalBank.
         *
         * Default is fs_ / nSamples_
         *
         * @return The frame rate (Hz).
         */
        Real getFrameRate() const;

        /**
         * @brief Returns the state of the SignalBank.
         *
         * @return true if initialised, false otherwise.
         */
        bool isInitialized() const;

    private:

        int nChannels_, nSamples_;
        bool trig_, initialized_;
        int fs_;
        Real frameRate_;
        RealVecVec signal_;
        RealVec centreFreqs_;
    }; 
}
#endif 

