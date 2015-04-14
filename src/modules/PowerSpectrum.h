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

#ifndef POWERSPECTRUM_H
#define POWERSPECTRUM_H

#include "../support/Module.h"
#include "../support/FFT.h"

namespace loudness{

    /**
     * @class PowerSpectrum
     *
     * @brief Computes the power spectrum of an input SignalBank.  
     *
     * A multi-resolution power spectrum can be obtained by constructing the
     * object with a vector of non-overlapping frequency bands and a vector of
     * window lengths corresponding to each band. The band edges should be
     * specified in Hz and the window sizes in samples.
     *
     * For example:
     *  RealVec bandFreqs{20, 100, 500};
     *  RealVec windowSpec{2048, 1024};
     *  PowerSpectrum object(bandFreqs, windowSpec, true)
     *
     * will construct a PowerSpectrum object which uses a window length of 2048
     * samples for all frequency bins in the interval [20, 100) and a 1024 for
     * all bins in the interval [100, 500). Thus the input SignalBank should
     * hold a 2048 frames worth of input samples in two separate channels. The
     * number of input channels must match the number of windows!  In this
     * example, all 2048 samples in channel one will be used to construct the
     * first band, and the first 1024 samples of channel two will be used for
     * the second band (the remaining 1024 samples are ignored). The idea is
     * that this module can be used in conjunction with \ref Window which can
     * output multiple windowed data frames. 
     *
     * The boolean argument sampleSpectrumUniformly determines whether all DFT bands
     * are sampled uniformly.  If sampleSpectrumUniformly is false, the per band
     * spectrum is sampled non-sampleSpectrumUniformlyly at intervals corresponding
     * to fs/FFTsize, where FFTsize depends on the size of each window.
     *
     * In the current implementation, a Hann window is applied to all segments
     * and both DC and Nyquist are not computed.
     *
     * @sa FrameGenerator, Window
     */
    class PowerSpectrum: public Module
    {
    public:

        /**
         * @brief Constructs a PowerSpectrum object.
         *
         * @param bandFreqsHz A vector of consecutive band edges in Hz.
         * @param windowSizes A vector of window sizes for each band in samples.
         * @param sampleSpectrumUniformly Set true to use the sample FFT size for
         * each window.
         */
        PowerSpectrum(const RealVec& bandFreqsHz,
                const vector<int>& windowSizes, 
                bool sampleSpectrumUniformly);

        virtual ~PowerSpectrum();

        void setNormalisation(const string &normalisation);

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();

        RealVec bandFreqsHz_, normFactor_;
        vector<int> windowSizes_;
        bool sampleSpectrumUniformly_;
        string normalisation_;
        vector<vector<int> > bandBinIndices_; 
        vector<unique_ptr<FFT>> ffts_;
    };
}

#endif
