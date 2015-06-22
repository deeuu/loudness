#include "HoppingGoertzelDFT.h"

namespace loudness{
    
    HoppingGoertzelDFT::HoppingGoertzelDFT(Real loFrequency,
                Real hiFrequency,
                int windowSize,
                int hopSize) :
            Module("HoppingGoertzelDFT"),
            loFrequency_(loFrequency),
            hiFrequency_(hiFrequency),
            windowSize_(windowSize),
            hopSize_(hopSize)
    {}

    HoppingGoertzelDFT::~HoppingGoertzelDFT() {}

    bool HoppingGoertzelDFT::initializeInternal(const SignalBank &input)
    {   
        // Sampling frequency
        int fs = input.getFs();

        // These are NOT the nearest components but satisfies f_k in [f_lo, f_hi)
        int binOfLoFreq = std::ceil (loFrequency_ * windowSize_ / fs);
        // use < binOfHiFreq to exclude f_hi
        int binOfHiFreq = std::ceil (hiFrequency_ * windowSize_ / fs);
        LOUDNESS_DEBUG(name_ << ": binOfLoFreq: " << binOfLoFreq);
        LOUDNESS_DEBUG(name_ << ": binOfHiFreq: " << binOfHiFreq);

        if (binOfHiFreq == 0)
        {
            LOUDNESS_ERROR (name_ << ": No components found in band number ");
            return 0;
        }
        if (binOfHiFreq <= binOfLoFreq)
        {
            LOUDNESS_ERROR (name_ << ": upper band edge is not > lower band edge!");
            return 0;
        }

        // initialise the delay line
        delayLine_.assign (input.getNEars(), RealVec (windowSize_, 0.0));

        // total number of bins in the output spectrum
        int nBins = binOfHiFreq - binOfLoFreq;

        LOUDNESS_DEBUG (name_ 
                << ": Total number of bins comprising the spectrum: " 
                << nBins);

        // Goertzel filter variables
        sine_.assign (nBins, 0.0);
        cosineTimes2_.assign (nBins, 0.0);
        vPrev_.assign (input.getNEars(), RealVec (nBins, 0.0));
        vPrev2_.assign (input.getNEars(), RealVec (nBins, 0.0));

        // output bank
        output_.initialize (input.getNEars(), nBins, 2, fs);
        output_.setFrameRate (fs / (Real)hopSize_);
        output_.setTrig (false);

        int i = 0;
        for (int j = binOfLoFreq; j < binOfHiFreq; ++j, ++i)
        {
            // bin frequency in Hz
            output_.setCentreFreq (i, j * fs / (Real)windowSize_);

            // filter coefficients
            Real phi = 2.0 * PI * j / (Real)windowSize_;
            Real sinPhi = sin (phi);
            Real cosPhi = cos (phi);
            sine_[i] = sinPhi;
            cosineTimes2_[i] = 2.0 * cosPhi;
        }

        writeIdx_ = 0;
        nSamplesUntilTrigger_ = windowSize_;

        return 1;
    }

    void HoppingGoertzelDFT::processInternal(const SignalBank &input)
    {
        //output trigger
        output_.setTrig (false);

        for (int ear = 0; ear < input.getNEars(); ++ear)
        {
            int nRemainingSamples = input.getNSamples();
            int nSamplesToProcess = 0;

            // number of samples to process until DFT is ready
            if (nSamplesUntilTrigger_ > nRemainingSamples)
                nSamplesToProcess = nRemainingSamples;
            else
                nSamplesToProcess = nSamplesUntilTrigger_;

            while (nRemainingSamples)
            {
                for (int ear = 0; ear < input.getNEars(); ++ear)
                {
                    const Real* x = input.getSignalReadPointer(ear, 0, 0);
                    for (int smp = 0; smp < nSamplesToProcess; ++smp)
                    {
                        // the comb filter
                        Real comb = x[smp] - delayLine_[ear][writeIdx_];

                        // write to the delay line
                        delayLine_[ear][writeIdx_] = x[smp];

                        // compute v[n] for each frequency
                        for (int k = 0; k < output_.getNChannels(); ++k)
                        {
                            Real v = comb + cosineTimes2_[k] * 
                                     vPrev_[ear][k] - vPrev2_[ear][k];
                            vPrev2_[ear][k] = vPrev_[ear][k];
                            vPrev_[ear][k] = v;
                        }
                        writeIdx_ = (writeIdx_ + 1) % windowSize_;
                    }
                }

                nSamplesUntilTrigger_ -= nSamplesToProcess;

                // compute the complex-real multiplication
                if(nSamplesUntilTrigger_ == 0)
                {
                    // write to the output signal bank using a single pointer
                    Real* y = output_.getSignalWritePointer(0, 0, 0);
                    for (int ear = 0; ear < input.getNEars(); ++ear)
                    {
                        for (int k = 0; k < output_.getNChannels(); ++k)
                        {
                            // real
                            *(y++) = 0.5 * cosineTimes2_[k] * vPrev_[ear][k] - vPrev2_[ear][k];
                            // imag
                            *(y++) = sine_[k] * vPrev_[ear][k];	
                        }
                    }
                    nSamplesUntilTrigger_ = hopSize_;
                    output_.setTrig (true);
                }
                nRemainingSamples -= nSamplesToProcess;
                nSamplesToProcess = nRemainingSamples;
            }
        }
    }

    void HoppingGoertzelDFT::resetInternal()
    {
        for (uint ear = 0; ear < vPrev_.size(); ++ear)
        {
            for (uint smp = 0; smp < delayLine_[ear].size(); ++smp)
            {
                delayLine_[ear][smp] = 0.0;
            }

            for (uint k = 0; k < vPrev_[ear].size(); ++k)
            {
                vPrev_[ear][k] = 0.0;
                vPrev2_[ear][k] = 0.0;
            }
        }
        output_.setTrig (false);
    }
}
