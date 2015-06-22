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
     * difference equations:
     *
     *  v[n] = x[n] - x[n-N] + 2 cos(2 pi k / N) v[n-1] - v[n-2]
     *  y[n] = e^(j 2 pi k / N) v[n] - v[n-1]
     *
     * where x and y are the input and output signals, k is the spectral bin
     * index and N is the transform length.  Note however, that the
     * complex-by-real multiplication giving y[n], need only be computed every
     * hop samples. When the hop size is > 1, we get the Hoping Goertzel DFT.
     *
     * This module allows you to compute the DFT of select frequencies from a
     * specified frequency band.
     * The band edges (in Hz), window length (in samples) and hop size (in
     * samples) must be specified.
     *
     *  Example:
     *  HoppingGoertzelDFT object(50, 1000, 1024, 512)
     *
     * Will construct a HoppingGoertzelDFT object which uses a transform length
     * of 1024 samples for all DFT bins in the interval [50~Hz, 1000~Hz).
     * The complex spectrum is updated every 512 samples.
     */

    class HoppingGoertzelDFT : public Module
    {

    public:
        HoppingGoertzelDFT(Real loFrequency,
                Real hiFrequency,
                int windowSize,
                int hopSize);

        virtual ~HoppingGoertzelDFT();

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();

        Real loFrequency_, hiFrequency_;
        int windowSize_, hopSize_;
        int nSamplesUntilTrigger_, writeIdx_;
        RealVec sine_, cosineTimes2_;
        RealVecVec vPrev_, vPrev2_;
        RealVecVec delayLine_;
    };
}

#endif
