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

#include <fftw3.h>
#include "../Support/Module.h"

/*
 * =====================================================================================
 *                Notes:
 *
 *                - The power spectrum is computed following:
 *                http://www.dadisp.com/webhelp/dsphelp.htm#mergedprojects/refman2/SPLGROUP/POWSPEC.htm
 *                such that:
 *                sum(power_spec(x)) = mean(x*x)
 *                Thus the area under the curve represents the average power of x, rather than the total power of x.
 *
 *                - In the current implementation, DC and nyquist are not computed.
 *
 *                - The windows should be centred at the same sample point ((window_size-1)/2).
 *                Only if the window size is odd will this will be an integer.
 *                Note that with the multi-resolution specification of Glasberg and Moore,
 *                at fs=44100, we have a mixture of windows odd and even length windows, so technically
 *                they are not perfectly alined. To remedy this, one could force all window lengths to be
 *                odd, but I have not implemented this - it should not be a major issue.
 *
 *                - only bin frequencies (f_k) in the interval f_lo <= f_k < f_hi are included, where
 *                f_lo and f_hi denote the frequencies of the band edges.
 *                - when different fft sizes are used, the band configuration can be automatically adjusted 
 *                to maintain proximity between bins where possible by commenting out code in the cpp source.
 *                This does mean however that the [f_lo, f_hi) criterion is not satisfied.
 *
 *                TODO(dominic.ward@bcu.ac.uk):  
 *                - Develop Window class to allow windows other than hann.
 *  
 * =====================================================================================
 */

namespace loudness{

    /**
     * @class PowerSpectrum
     * @brief Computes the power spectrum (PS) of an input SignalBank.  A
     * mulit-resolution PowerSpectrum can be obtained by specifying different
     * frequency bands along with the corresponding window lengths.  When the
     * FFT size is greater than the window size, casual zero padding is used.
     * All windows are aligned at their temporal centered.
     *
     * It is advised that the input SignalBank has the correct frameRate when
     * initialising this object.
     */
    class PowerSpectrum: public Module
    {
    public:

        PowerSpectrum(const RealVec& bandFreqsHz, const RealVec& windowSizeSecs, bool uniform);

        virtual ~PowerSpectrum();

    private:

        virtual bool initializeInternal(const SignalBank &input);

        virtual void processInternal(const SignalBank &input);

        virtual void resetInternal();

        void hannWindow(RealVec &w, int fft_size);

        RealVec bandFreqsHz_, windowSizeSecs_;
        bool uniform_;
        int nWindows_;
        Real temporalCentre_;
        Real *fftInputBuf_, *fftOutputBuf_;
        vector<int> windowSizeSamps_, fftSize_, windowDelay_;
        vector<fftw_plan> fftPlans_;
        RealVecVec windows_;
        vector<vector<int> > bandBinIndices_; 
    };
}

#endif

