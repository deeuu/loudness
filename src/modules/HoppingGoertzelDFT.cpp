#include "HoppingGoertzelDFT.h"

namespace loudness{
    
    HoppingGoertzelDFT::HoppingGoertzelDFT(const RealVec& frequencyBandEdges,
                const vector<int>& windowSizes,
                int hopSize,
                bool isHannWindowUsed,
                bool isPowerSpectrum) :
            Module("HoppingGoertzelDFT"),
            frequencyBandEdges_(frequencyBandEdges),
            windowSizes_ (windowSizes),
            hopSize_ (hopSize),
            isHannWindowUsed_ (isHannWindowUsed),
            isPowerSpectrum_ (isPowerSpectrum),
            reference_ (2e-5)
    {}

    HoppingGoertzelDFT::~HoppingGoertzelDFT() {}

    bool HoppingGoertzelDFT::initializeInternal(const SignalBank &input)
    {   
        // Sampling frequency
        int fs = input.getFs();

        nWindows_ = windowSizes_.size();
        // bin indices of components at the band edges
        vector< vector<int>> bandEdgeIdx (nWindows_, vector<int> (2, 0));
        largestWindowSize_ = 0;
        int nBins = 0, nAdditionalBins = 0;
        for (int w = 0; w < nWindows_; ++w)
        {
            // These are NOT the nearest components but satisfies f_k in [f_lo, f_hi)
            bandEdgeIdx[w][0] = std::ceil (frequencyBandEdges_[w] * windowSizes_[w] / fs);
            // use < bandEdgeIdx[w][1] to exclude f_hi
            bandEdgeIdx[w][1] = std::ceil (frequencyBandEdges_[w + 1] * windowSizes_[w] / fs);
            LOUDNESS_DEBUG(name_ << ": LoFreq: " << bandEdgeIdx[w][0] * fs / (Real)windowSizes_[w]);
            LOUDNESS_DEBUG(name_ << ": HiFreq: " << bandEdgeIdx[w][1] * fs / (Real)windowSizes_[w]);
            int highestBin = windowSizes_[w] / 2;

            LOUDNESS_ASSERT(bandEdgeIdx[w][1] != 0, ": No components found in band number ");
            LOUDNESS_ASSERT(bandEdgeIdx[w][1] >= bandEdgeIdx[w][0],
                ": upper band edge is not > lower band edge!");
            LOUDNESS_ASSERT(bandEdgeIdx[w][1] <= highestBin,
                    ": upper band edge is > highest positive frequency.");

            nBins += bandEdgeIdx[w][1] - bandEdgeIdx[w][0];
            if (windowSizes_[w] > largestWindowSize_)
                largestWindowSize_ = windowSizes_[w];

            // windowing constraints
            if (isHannWindowUsed_)
            {
                if (bandEdgeIdx[w][0] == 0)
                {
                    LOUDNESS_ERROR (name_ 
                            << ": This implementation does not support windowing with DC.");
                    return 0;
                }
                if (bandEdgeIdx[w][1] == highestBin)
                {
                    LOUDNESS_ERROR (name_
                            << ": This implementation does not support windowing with the highest positive frequency.");
                    return 0;
                }
                bandEdgeIdx[w][0] -= 1;
                bandEdgeIdx[w][1] += 1;
                nAdditionalBins += 2;
            }

            LOUDNESS_DEBUG(name_
                << ": bin lo : " << bandEdgeIdx[w][0]
                << ", bin hi : " << bandEdgeIdx[w][1]);
        }

        int nTotalBins = nBins + nAdditionalBins;
        LOUDNESS_DEBUG (name_ 
                << ": Total number of bins comprising the spectrum: " 
                << nBins
                << ": Total number os bins used by this algorithm: "
                << nTotalBins);

        // in order to simplify the alignment of windows, make the delay line an
        // integer multiple of the input block size.
        int blockSize = input.getNSamples();
        delayLineSize_ = blockSize * (std::ceil (largestWindowSize_ / input.getNSamples()) + 1);
        LOUDNESS_DEBUG(name_ << ": delay line size: " << delayLineSize_);
        delayLine_.assign (input.getNEars(), RealVec (delayLineSize_, 0.0));
        // read points for time n and n - windowSizes_[w]
        readIdx_.assign (nWindows_, vector<int> (2, 0));
        configureDelayLineIndices();

        // Goertzel filter variables
        sine_.assign (nTotalBins, 0.0);
        cosineTimes2_.assign (nTotalBins, 0.0);
        vPrev_.assign (input.getNEars(), RealVec (nTotalBins, 0.0));
        vPrev2_.assign (input.getNEars(), RealVec (nTotalBins, 0.0));
        binIdxForGoertzels_.assign (nWindows_, vector<int> (2, 0));

        // output bank
        if (isPowerSpectrum_)
        {
            output_.initialize (input.getNEars(), nBins, 1, fs);
            normFactors_.assign (nWindows_, 1.0);
        }
        else
        {
            output_.initialize (input.getNEars(), nBins, 2, fs);
        }
        output_.setFrameRate (fs / (Real)hopSize_);
        output_.setTrig (false);

        // Complete arrays
        int k = 0, k2 = 0;
        for (int w = 0; w < nWindows_; ++w)
        {
            binIdxForGoertzels_[w][0] = k;
            for (int j = bandEdgeIdx[w][0]; j < bandEdgeIdx[w][1]; ++j, ++k)
            {
                // bin frequency in Hz
                if (isHannWindowUsed_)
                {
                    if ((j > bandEdgeIdx[w][0]) && (j < (bandEdgeIdx[w][1] - 1)))
                        output_.setCentreFreq (k2++, j * fs / (Real)windowSizes_[w]);
                }
                else
                    output_.setCentreFreq (k2++, j * fs / (Real)windowSizes_[w]);

                // filter coefficients
                Real phi = 2.0 * PI * j / (Real)windowSizes_[w];
                Real sinPhi = sin (phi);
                Real cosPhi = cos (phi);
                sine_[k] = sinPhi;
                cosineTimes2_[k] = 2.0 * cosPhi;
            }
            binIdxForGoertzels_[w][1] = k;
            if (isPowerSpectrum_)
            {
                Real refSquared = reference_ * reference_;
                Real windowSizeSquared = windowSizes_[w] * windowSizes_[w];
                normFactors_[w] = 2.0 / (refSquared * windowSizeSquared);
                // 3/8 for hann window and 0.0625 for gain of 16 introduced by
                // selected coefficients
                if (isHannWindowUsed_)
                    normFactors_[w] *= (3.0 * 0.0625) / 8.0;
            }
        }

        nSamplesUntilTrigger_ = largestWindowSize_;
        tempNSamplesUntilTrigger_ = nSamplesUntilTrigger_;

        return 1;
    }

    void HoppingGoertzelDFT::processInternal(const SignalBank &input)
    {
        output_.setTrig (false);

        int nSamplesToProcess = input.getNSamples();
        int nRemainingSamples = nSamplesToProcess;

        // Write to the delay line
        for (int ear = 0; ear < input.getNEars(); ++ear)
        {
            const Real* x = input.getSignalReadPointer(ear, 0, 0);
            for (int smp = 0; smp < nSamplesToProcess; ++smp)
                delayLine_[ear][writeIdx_++] = x[smp];
            writeIdx_ = writeIdx_ % delayLineSize_;
        }

        // number of samples to process until DFT is ready
        if (tempNSamplesUntilTrigger_ <= nRemainingSamples)
            nSamplesToProcess = tempNSamplesUntilTrigger_;

        // Goertzel resonators
        while (nRemainingSamples)
        {
            for (int ear = 0; ear < input.getNEars(); ++ear)
            {
                for (int w = 0; w < nWindows_; ++w)
                {
                    for (int smp = 0; smp < nSamplesToProcess; ++smp)
                    {
                        Real comb = delayLine_[ear][readIdx_[w][0]]
                            - delayLine_[ear][readIdx_[w][1]];
                        readIdx_[w][0] = (readIdx_[w][0] + 1) % delayLineSize_;
                        readIdx_[w][1] = (readIdx_[w][1] + 1) % delayLineSize_;

                        // compute v[n] for each frequency
                        for (int k = binIdxForGoertzels_[w][0]; k < binIdxForGoertzels_[w][1]; ++k)
                        {
                            Real v = comb + cosineTimes2_[k] * 
                                     vPrev_[ear][k] - vPrev2_[ear][k];
                            vPrev2_[ear][k] = vPrev_[ear][k];
                            vPrev_[ear][k] = v;
                        }
                    }
                }
            }

            tempNSamplesUntilTrigger_ -= nSamplesToProcess;

            // compute the complex-real multiplication
            if(tempNSamplesUntilTrigger_ == 0)
            {
                if (isHannWindowUsed_)
                {
                    if (isPowerSpectrum_)
                        calculatePowerSpectrumAndApplyHannWindow(input.getNEars());
                    else
                        calculateSpectrumAndApplyHannWindow(input.getNEars());
                }
                else
                {
                    if (isPowerSpectrum_)
                        calculatePowerSpectrum(input.getNEars());
                    else
                        calculateSpectrum(input.getNEars());
                }
                
                tempNSamplesUntilTrigger_ = hopSize_;
                output_.setTrig (true);
            }
            nRemainingSamples -= nSamplesToProcess;
            nSamplesToProcess = nRemainingSamples;
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
        tempNSamplesUntilTrigger_ = nSamplesUntilTrigger_;
        configureDelayLineIndices();
    }

    void HoppingGoertzelDFT::configureDelayLineIndices()
    {
        writeIdx_ = 0;
        for (int w = 0; w < nWindows_; ++w)
        {
            readIdx_[w][0] = (delayLineSize_ - 
                    ((largestWindowSize_ - windowSizes_[w]) / 2)) %
                delayLineSize_;
            readIdx_[w][1] = (delayLineSize_ + readIdx_[w][0] - windowSizes_[w]) %
                delayLineSize_;
            LOUDNESS_DEBUG(name_ << ": window : "
                    << w
                    << ", start idx: "
                    << readIdx_[w][0]
                    << ", end idx: "
                    << readIdx_[w][1]);
        }
    }

    void HoppingGoertzelDFT::calculateSpectrum(int nEars)
    {
        for (int ear = 0; ear < nEars; ++ear)
        {
            int chn = 0;
            for (int w = 0; w < nWindows_; ++w)
            {
                for (int k = binIdxForGoertzels_[w][0]; k < binIdxForGoertzels_[w][1]; ++k)
                {
                    Real* y = output_.getSignalWritePointer(ear, chn++);
                    *y = 0.5 * cosineTimes2_[k] * vPrev_[ear][k] - vPrev2_[ear][k];
                    *(++y) = sine_[k] * vPrev_[ear][k];	
                }
            }
        }
    }

    void HoppingGoertzelDFT::calculatePowerSpectrum(int nEars)
    {
        for (int ear = 0; ear < nEars; ++ear)
        {
            Real* y = output_.getSingleSampleWritePointer (ear, 0);
            for (int w = 0; w < nWindows_; ++w)
            {
                for (int k = binIdxForGoertzels_[w][0]; k < binIdxForGoertzels_[w][1]; ++k)
                {
                    Real real = 0.5 * cosineTimes2_[k] * vPrev_[ear][k] - vPrev2_[ear][k];
                    Real imag = sine_[k] * vPrev_[ear][k];	
                    *(y++) = normFactors_[w] * (real * real + imag * imag);
                }
            }
        }
    }

    void HoppingGoertzelDFT::calculateSpectrumAndApplyHannWindow (int nEars)
    {
        for (int ear = 0; ear < nEars; ++ear)
        {
            int chn = 0;
            for (int w = 0; w < nWindows_; ++w)
            {
                int i = 0;
                RealVec reals(3, 0.0), imags(3, 0.0);
                for (int k = binIdxForGoertzels_[w][0]; k < binIdxForGoertzels_[w][1]; ++k)
                { 
                    reals[i] = 0.5 * cosineTimes2_[k] * vPrev_[ear][k] - vPrev2_[ear][k];
                    imags[i++] = sine_[k] * vPrev_[ear][k];	

                    if (i == 3)
                    {
                        Real* y = output_.getSignalWritePointer(ear, chn++);
                                     // X_k-1           X_k+1            X_k
                        *y =     (-0.25*reals[0] - 0.25*reals[2] + 0.5 * reals[1]);
                        *(++y) = (-0.25*imags[0] - 0.25*imags[2] + 0.5 * imags[1]);
                        reals[0] = reals[1];
                        reals[1] = reals[2];
                        imags[0] = imags[1];
                        imags[1] = imags[2];
                        i = 2;
                    }
                }
            }
        }
    }

    void HoppingGoertzelDFT::calculatePowerSpectrumAndApplyHannWindow (int nEars)
    {
        for (int ear = 0; ear < nEars; ++ear)
        {
            Real* y = output_.getSingleSampleWritePointer (ear, 0);
            for (int w = 0; w < nWindows_; ++w)
            {
                int i = 0;
                RealVec reals(3, 0.0), imags(3, 0.0);
                for (int k = binIdxForGoertzels_[w][0]; k < binIdxForGoertzels_[w][1]; ++k)
                { 
                    reals[i] = 0.5 * cosineTimes2_[k] * vPrev_[ear][k] - vPrev2_[ear][k];
                    imags[i++] = sine_[k] * vPrev_[ear][k];	

                    if (i == 3)
                    {
                                   // X_k-1      X_k+1            X_k
                        Real real = (-reals[0] - reals[2] + 2.0 * reals[1]);
                        Real imag = (-imags[0] - imags[2] + 2.0 * imags[1]);
                        *(y++) = normFactors_[w] * (real * real + imag * imag);
                        reals[0] = reals[1];
                        reals[1] = reals[2];
                        imags[0] = imags[1];
                        imags[1] = imags[2];
                        i = 2;
                    }
                }
            }
        }
    }

    void HoppingGoertzelDFT::setReference (Real reference)
    {
        reference_ = reference;
    }
}

