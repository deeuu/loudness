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
     * A multi-resolution power spectrum can be obtained by constructing an
     * object with a vector of non-overlapping frequency bands and a vector of
     * window lengths corresponding to each band. The band edges should be
     * specified in Hz and the window sizes in samples.
     *
     * For example:
     * @code
     *      RealVec bandFreqs{20, 100, 500};
     *      RealVec windowSpec{2048, 1024};
     *      PowerSpectrum object(bandFreqs, windowSpec, true);
     * @endcode
     *
     * will construct a PowerSpectrum object which uses a window length of 2048
     * samples for all frequency bins in the interval [20, 100) and a length of 1024 for
     * all bins in the interval [100, 500). The input SignalBank should
     * hold 2048 samples in two separate channels (yes 2048 for both). The
     * number of input channels must match the number of windows.  In this
     * example, all 2048 samples in channel one will be used to construct the
     * first band, and the first 1024 samples of channel two will be used for
     * the second band (the remaining 1024 samples are ignored). The idea is
     * that this module can be used in conjunction with \ref Window which can
     * output multiple windowed data frames. 
     *
     * The boolean argument @a sampleSpectrumUniformly determines whether all DFT bands
     * are sampled uniformly.  If @a sampleSpectrumUniformly is false, the per band
     * spectrum is sampled non-uniformly at intervals corresponding
     * to fs/FFTsize, where FFTsize depends on the size of each window.
     *
     * In the current implementation, DC and Nyquist are excluded.
     *
     * The one-sided power spectra can be normalised for energy, average power
     * or neither. Use setNormalisation() to change the default from
     * AVERAGE_POWER. See PowerSpectrum::Normalisation.
     *
     * A reference factor can be incorporated in to the normalisation (it is
     * squared). The default value is 2e-5 which corresponds to the common sound
     * pressure reference of 20 micro pascals. Use setReferenceValue() to change
     * this.
     *
     * @sa FrameGenerator, Window
     */
    class PowerSpectrum: public Module
    {
    public:
    
        enum Normalisation{
            NONE,
            ENERGY,
            AVERAGE_POWER
        };
 
        /**
         * @brief Constructs a PowerSpectrum object.
         *
         * @param bandFreqsHz A vector of band edges (in Hz) in ascending order.
         * @param windowSizes A vector of window sizes (in samples) for each band.
         * @param sampleSpectrumUniformly Set true to use the same FFT size for
         * all bands.
         */
        PowerSpectrum(const RealVec& bandFreqsHz,
                const vector<int>& windowSizes, 
                bool sampleSpectrumUniformly);

        virtual ~PowerSpectrum();

        void setNormalisation(const Normalisation normalisation);

        void setReferenceValue(Real referenceValue);

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();

        RealVec bandFreqsHz_, normFactor_;
        vector<int> windowSizes_;
        bool sampleSpectrumUniformly_;
        Normalisation normalisation_;
        Real referenceValue_;
        vector<vector<int> > bandBinIndices_; 
        vector<unique_ptr<FFT>> ffts_;
    };
}

#endif
