#ifndef HoppingGoertzelDFT_H
#define HoppingGoertzelDFT_H

#include "../support/Module.h"

namespace loudness{

    /**
     * @class HoppingGoertzelDFT
     * @brief Computes the DFT or PowerSpectrum of a band of frequencies using a
     * bank of second order Goertzel resonators.
     *
     * The sliding Goertzel DFT algorithm is implemented using the following
     * two difference equations:
     *
     * \f{eqnarray*}{
     *  v[n] &=& x[n] - x[n-N] + 2 cos(2 \pi k / N) v[n-1] - v[n-2] \\
     *  y[n] &=& e^{j 2 \pi k / N} v[n] - v[n-1]
     *  \f}
     *
     * where \f$x\f$ and \f$y\f$ are the input and output signals, \f$k\f$ is the spectral bin
     * index and \f$N\f$ is the transform length.  Note however, that the
     * complex-by-real multiplication giving \f$y[n]\f$, need only be computed every
     * hop samples. When the hop size is > 1, we get the Hoping Goertzel DFT
     * (rather than the sliding form).
     *
     * This module allows you to compute the DFT of select frequencies from
     * a number of frequency bands each with different transform lengths, i.e.,
     * a multi-resolution DFT.
     * The band edges (in Hz), window length (in samples) and hop size (in
     * samples) must be specified.
     *
     * Example:
     * @code
     *      vector<Real> bandFrequencies = {50, 1000, 5000};
     *      vector<int> windowSizes = {1024, 512};
     *      int hopSize = 256;
     *      HoppingGoertzelDFT object(bandFrequencies, windowSizes, hopSize, false, false);
     * @endcode
     *
     * will construct a HoppingGoertzelDFT object which uses a transform length
     * of 1024 samples for all DFT bins in the interval [50 Hz, 1000 Hz), and a
     * transform length of 512 samples for bins in the interval [1000 Hz,
     * 5000 Hz).  The complex spectrum is updated every 256 samples. The fourth
     * parameter (@a isHannWindowUsed) is a boolean that determines if the spectrum is
     * smoothed using a hann window in the frequency domain.
     * The fifth parameter (@a isPowerSpectrum) is a boolean that specifies whether the
     * power spectrum should be computed as opposed to outputting the complex
     * DFT coefficients.
     *
     * - The hop size must be equal to or greater than the number of samples in
     * the input SignalBank used to construct the object. 
     * - The window size must be equal to or greater than the hop size.
     * - In the current implementation, both DC and the highest bin (e.g.
     *   nyquist) are excluded when windowing is applied.
     * - The one-sided power spectrum is normalised for average power.
     * - A reference factor can be incorporated in to the normalisation (it is
     *   squared). The default value is 2e-5 which corresponds to the common sound
     *   pressure reference of 20 micro pascals. Use setReferenceValue() to change
     *   this.
     *
     *
     * Note: This module probably does too much. However, implementing a module
     * for a single band HGDFT starts to get messy when you require the windows
     * aligned at their temporal centres. It is easier (and more efficient) to
     * implement multiple DFT bands in a single module. Though the calculation
     * of the power spectrum should be pulled out as a seperate module, doing
     * so leads to code duplication (or interface code) and is thus prone to
     * errors.
     */

    class HoppingGoertzelDFT : public Module
    {

    public:

        HoppingGoertzelDFT(const RealVec& frequencyBandEdges,
                const vector<int>& windowSizes,
                int hopSize,
                bool isHannWindowUsed,
                bool isPowerSpectrum);
        virtual ~HoppingGoertzelDFT();
        void setReferenceValue (Real referenceValue);
        void setFirstSampleAtWindowCentre (bool isFirstSampleAtWindowCentre);

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();
        void configureDelayLineIndices();
        void calculateSpectrum();
        void calculatePowerSpectrum();
        void calculateSpectrumAndApplyHannWindow();
        void calculatePowerSpectrumAndApplyHannWindow();

        RealVec frequencyBandEdges_;
        vector<int> windowSizes_;
        int hopSize_;
        bool isHannWindowUsed_, isPowerSpectrum_, isFirstSampleAtWindowCentre_;
        Real referenceValue_;
        int nSamplesUntilTrigger_, writeIdx_;
        int nWindows_, delayLineSize_, largestWindowSize_;
        vector< vector<int>> binIdxForGoertzels_, readIdx_;
        RealVec sine_, cosineTimes2_, normFactors_;
        SignalBank delayLine_, vPrev_, vPrev2_;
    };
}

#endif
