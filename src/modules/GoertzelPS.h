#ifndef GOERTZELPS_H
#define GOERTZELPS_H

#include "../Support/Module.h"

namespace loudness{

    /**
     * @class GoertzelPS
     * @brief Computes a multi-resolution power spectrum using a bank of Goertzel resonators.
     *
     * The sliding Goertzel DFT algorithm is implemented using the following
     * difference equations:
     *
     *  v[n] = x[n] - x[n-N] + 2 cos(2 pi k / N) v[n-1] - v[n-2]
     *  y[n] = e^(j 2 pi k / N) v[n] - v[n-1]
     *
     * where x and y are the input and output respecitvely, k is the spectral bin index and N is the transform length.
     * Note however, that the complex-by-real multiplication giving y[n], need only be computed every hop
     * samples. When the hop size is > 1, we get the Hoping Goertzel DFT.
     *
     * A multi-resolution spectrum can be obtained by tuning a set of sliding
     * Goertzels to specific freqeuency points within multiple bands of varying
     * transform lengths. The band edges (in Hz) corresponding each frequency
     * band, along with the window length corresponding to that band need to be
     * specified.
     *
     * For example:
     *  RealVec bandFreqs{20, 100, 500};
     *  RealVec windowSpec{0.064, 0.032};
     *  GoertzelPS object(bandFreqs, windowSpec, 0.001)
     *
     * Will construct a GoertzelPS object which uses a 64ms window for all
     * frequency bins in the interval [20, 100) and a 32ms window for all bins
     * in the interval [100, 500). The windows are all time-aligned at their
     * centre points, and the composite spectrum is updated at 1~ms intervals.
     *
     * Windowing is performed in the frequency domain according to:
     *  X_windowed[b, k] = - X[b, k-1] + 2*X[b, k] - X[b, k+1]
     *
     * where b is the band number. This introduces a gain of four.
     *
     * Each spectrum is scaled such that the sum of component powers in a given
     * band equals the average power in that band (see,
     * http://www.dadisp.com/webhelp/dsphelp.htm#mergedprojects/refman2/SPLGROUP/POWSPEC.htm).
     * The spectra are also compensated for the gain introduced by hann windowing.
     *
     * In the current implementation, DC and nyquist are not computed.
     *
     * @author Dominic Ward
     */

    /*
     * Have checked the resulting power spectrum and gives same result as
     * PowerSpectrum when no windowing is used.  However, frequency domain
     * windowing seems to give small deviations at low levels which I attribute to
     * numerical differences. See the source for PowerSpectrum for further notes.
    */

    class GoertzelPS : public Module
    {

    public:
        GoertzelPS(const RealVec& bandFreqsHz, const RealVec& windowSizeSecs, Real hopSizeSecs);
        virtual ~GoertzelPS();
        void setWindowSpectrum(bool windowSpectrum);

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual void processInternal(const SignalBank &input);
        virtual void resetInternal();
        void windowedPS();
        void computePS();
        void configureDelays();

        RealVec bandFreqsHz_, windowSizeSecs_;
        Real hopSizeSecs_, temporalCentre_;
        RealVecVec sine_, cosineTimes2_, vPrev_, vPrev2_;
        RealVec delayLine_, norm_;
        vector<int> windowSizeSamps_, startIdx_, endIdx_;
        int nWindows_, hopSize_, delayLineSize_, delayWriteIdx_;
        int count_, maxCount_, initFrameReady_, frameReady_, largestWindowSize_;
        bool windowSpectrum_;
    };
}

#endif
