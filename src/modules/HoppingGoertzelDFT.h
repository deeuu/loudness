#ifndef HoppingGoertzelDFT_H
#define HoppingGoertzelDFT_H

#include "../support/Module.h"

namespace loudness{

    /**
     * @class HoppingGoertzelDFT
     * @brief Computes the DFT of a band of frequencies using a bank of second
     * order Goertzel resonators.
     *
     * The sliding Goertzel DFT algorithm is implemented using the following
     * two difference equations:
     *
     *  v[n] = x[n] - x[n-N] + 2 cos(2 pi k / N) v[n-1] - v[n-2]
     *  y[n] = e^(j 2 pi k / N) v[n] - v[n-1]
     *
     * where x and y are the input and output signals, k is the spectral bin
     * index and N is the transform length.  Note however, that the
     * complex-by-real multiplication giving y[n], need only be computed every
     * hop samples. When the hop size is > 1, we get the Hoping Goertzel DFT
     * (rather than the sliding form).
     *
     * This module allows you to compute the DFT of select frequencies from
     * a number of frequency bands each with different transform lengths, i.e.,
     * a multi-resolution DFT.
     * The band edges (in Hz), window length (in samples) and hop size (in
     * samples) must be specified.
     *
     *  Example:
     *  vector<Real> bandFrequencies = {50, 1000, 5000};
     *  vector<int> windowSizes = {1024, 512};
     *  int hopSize = 256;
     *  HoppingGoertzelDFT object(bandFrequencies, windowSizes, hopSize, false)
     *
     * Will construct a HoppingGoertzelDFT object which uses a transform length
     * of 1024 samples for all DFT bins in the interval [50~Hz, 1000~Hz), and a
     * transform length of 512 samples for bins in the interval [1000~Hz,
     * 5000~Hz).  The complex spectrum is updated every 256 samples. The last
     * parameter (false) is a booleon that determines if the spectrum is
     * smoothed using a hann window in the frequency domain. If this is true,
     * the hann windowed spectrum will have a gain of four applied as a result
     * of the simplified hann window coefficients used for the convolution.
     *
     * - The hop size must be equal to or greater than the number of samples in
     * the input SignalBank used to construct the object. 
     * - The window size must be equal to or greater than the hop size.
     * - If isHannWindowUsed is true, the spectral coefficients are increased by
     *   a factor of four as a result of the windowing procedure. 
     */

    class HoppingGoertzelDFT : public Module
    {

    public:
        HoppingGoertzelDFT(const RealVec& frequencyBandEdges,
                const vector<int>& windowSizes,
                int hopSize,
                bool isHannWindowUsed);
        virtual ~HoppingGoertzelDFT();

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();
        void configureDelayLineIndices();
        void calculateSpectrum(int nEars);
        void calculateSpectrumAndApplyHannWindow(int nEars);

        RealVec frequencyBandEdges_;
        vector<int> windowSizes_;
        int hopSize_;
        bool isHannWindowUsed_;
        int nSamplesUntilTrigger_, tempNSamplesUntilTrigger_, writeIdx_;
        int nWindows_, delayLineSize_, largestWindowSize_;
        vector< vector<int>> binIdxForGoertzels_, readIdx_;
        RealVec sine_, cosineTimes2_;
        RealVecVec vPrev_, vPrev2_;
        RealVecVec delayLine_;
    };
}

#endif
