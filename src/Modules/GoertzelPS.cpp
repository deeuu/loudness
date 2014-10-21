#include "GoertzelPS.h"

namespace loudness{
    
    GoertzelPS::GoertzelPS(const RealVec& bandFreqsHz,
            const RealVec& windowSizeSecs,
            Real hopSizeSecs) :
            Module("GoertzelPS"),
            bandFreqsHz_(bandFreqsHz),
            windowSizeSecs_(windowSizeSecs),
            hopSizeSecs_(hopSizeSecs),
            windowSpectrum_(true)
    {}

    GoertzelPS::~GoertzelPS() {}

    void GoertzelPS::setWindowSpectrum(bool windowSpectrum)
    {
        windowSpectrum_ = windowSpectrum;
    }

    bool GoertzelPS::initializeInternal(const SignalBank &input)
    {   
        //number of windows
        nWindows_ = (int)windowSizeSecs_.size();

        //check input
        if(bandFreqsHz_.size() != (windowSizeSecs_.size()+1))
        {
            LOUDNESS_ERROR(name_
                    << ": Number of frequency bands should equal number of windows + 1");
            return 0;
        }

        //fs
        int fs = input.getFs();

        //window size in samples
        windowSizeSamps_.resize(nWindows_);
        largestWindowSize_=0;
        for(int i=0; i<nWindows_; i++)
        {
            windowSizeSamps_[i] = round(fs*windowSizeSecs_[i]);
            LOUDNESS_DEBUG(name_
                    << ": Window size in samples: " 
                    << windowSizeSamps_[i]);
            if(windowSizeSamps_[i]>largestWindowSize_)
                largestWindowSize_ = windowSizeSamps_[i];
        }
        LOUDNESS_DEBUG(name_ 
                << ": Largest window size: " 
                << largestWindowSize_);

        //for this implementation, input buffer size must be smaller than largest window
        if(input.getNSamples() > largestWindowSize_)
        {
            LOUDNESS_ERROR(name_
                    << ": Number of input samples is: " 
                    << input.getNSamples() 
                    << " but must be <= largest window size:" 
                    << largestWindowSize_);
            return 0;
        }

        //hop size must be integer multiple of audio block size
        hopSize_ = round(fs*hopSizeSecs_);
        int blockSize = input.getNSamples();
        if(hopSize_<blockSize)
        {
            LOUDNESS_DEBUG(name_
                    << ": Hop size is less than "
                    << "input buffer size, automatically correcting...");

        }
        else if(0!=(hopSize_%blockSize))
        {
            LOUDNESS_DEBUG(name_
                    << ": Hop size is not a multiple of input "
                    << "buffer size, automatically correcting...");
        }
        hopSize_ = blockSize*ceil(hopSize_/(Real)blockSize);
        LOUDNESS_DEBUG(name_
                << ": Hop size in samples: " << hopSize_);

        //initialize the delay line make integer multiple of blockSize
        delayLineSize_ = blockSize*ceil(largestWindowSize_/(Real)blockSize) + hopSize_;
        LOUDNESS_DEBUG(name_
                << ": Delay line size: "
                << delayLineSize_);
        
        //binLimits contains the desired bins indices (lo and hi) per band
        vector<vector<int> > bandBinIndices(nWindows_);

        //appropriate delay for temporal alignment
        temporalCentre_ = (largestWindowSize_-1)/2.0;

        LOUDNESS_DEBUG(name_
                << ": Temporal centre of largest window: " 
                << temporalCentre_);

        //delays for windows
        configureDelays();

        norm_.resize(nWindows_);
        for(int i=0; i<nWindows_; i++)
        {
            //bin indices to use for compiled spectrum
            bandBinIndices[i].resize(2);
            //These are NOT the nearest components but satisfies f_k in [f_lo, f_hi)
            bandBinIndices[i][0] = ceil(bandFreqsHz_[i]*windowSizeSamps_[i]/fs);
            bandBinIndices[i][1] = ceil(bandFreqsHz_[i+1]*windowSizeSamps_[i]/fs)-1;
            if(bandBinIndices[i][1]==0)
            {
                LOUDNESS_ERROR(name_ << ": No components found in band number " << i);
                return 0;
            }

            //exclude DC and Nyquist if found
            if(bandBinIndices[i][0]==0)
            {
                LOUDNESS_WARNING(name_ << " : DC found...excluding.");
                bandBinIndices[i][0] = 1;
            }
            if(bandBinIndices[i][1] >= (windowSizeSamps_[i]/2.0))
            {
                LOUDNESS_WARNING(name_ << ": Bin is >= nyquist...excluding.");
                bandBinIndices[i][1] = (ceil(windowSizeSamps_[i]/2.0)-1);
            }

            //window normalisation
            //8/3 for hann
            //0.0625 for power gain of 16 due to freq domain windowing
            //2 for one sided spectrum
            norm_[i] = (2*0.0625*8)/(3.0*2e-5*2e-5*windowSizeSamps_[i]*windowSizeSamps_[i]);
        }

        //ensure no overlap
        int nBins = 0;
        for(int i=1; i<nWindows_; i++)
        {
            Real f1 = (bandBinIndices[i][0]*fs/windowSizeSamps_[i]);
            Real f2 = (bandBinIndices[i-1][1]*fs/windowSizeSamps_[i-1]);

            while( f1 <= f2)
                bandBinIndices[i][0] += 1;

            //this line will alter the band frequencies slightly to ensure closely spaced bins
            /*
             while(((bandBinIndices[i-1][1]+1)*fs/windowSizeSamps_[i-1]) 
                     < (bandBinIndices[i][0]*fs/windowSizeSamps_[i]))
             {
                bandBinIndices[i-1][1] += 1;   
             }
             */
            nBins += bandBinIndices[i-1][1]-bandBinIndices[i-1][0] + 1;
        }

        //total number of bins in the output spectrum
        nBins += bandBinIndices[nWindows_-1][1]-bandBinIndices[nWindows_-1][0] + 1;

        LOUDNESS_DEBUG(name_ 
                << ": Total number of bins comprising the spectrum: " 
                << nBins);

        #if defined(DEBUG)
        for(int i=0; i<nWindows_; i++)
        {
            Real edgeLo = bandBinIndices[i][0]*fs/(Real)windowSizeSamps_[i];
            Real edgeHi = bandBinIndices[i][1]*fs/(Real)windowSizeSamps_[i];
            LOUDNESS_DEBUG(name_ 
                    << ": Band interval (Hz) for Window of size: " 
                    << windowSizeSamps_[i]
                    << " = [ " << edgeLo << ", " 
                    << edgeHi << " ].");
        }
        #endif
        
        //Goertzel filter variables
        sine_.resize(nWindows_);
        cosineTimes2_.resize(nWindows_);
        vPrev_.resize(nWindows_);
        vPrev2_.resize(nWindows_);

        //output bank
        output_.initialize(nBins, 1, fs);
        output_.setFrameRate(fs/(Real)hopSize_);

        //fill in variables and compute centre frequencies
        int k=0;
        for(int i=0; i<nWindows_; i++)
        {
            for(int j=bandBinIndices[i][0]; j<=bandBinIndices[i][1]; j++)
            {
                output_.setCentreFreq(k++, j*fs/(Real)windowSizeSamps_[i]);
            }

            //2 redundant bins per band required for convolution
            bandBinIndices[i][0] -= 1;
            bandBinIndices[i][1] += 1;

            //filter coefficients
            for(int j=bandBinIndices[i][0]; j<=bandBinIndices[i][1]; j++)
            {
                Real phi = 2*PI*j/(Real)windowSizeSamps_[i];
                Real sinPhi = sin(phi);
                Real cosPhi = cos(phi);
		sine_[i].push_back(sinPhi);
                cosineTimes2_[i].push_back(2*cosPhi); 
                vPrev_[i].push_back(0.0);
                vPrev2_[i].push_back(0.0);

                LOUDNESS_DEBUG(name_ 
                        << ": Freq: " << j*fs/(Real)windowSizeSamps_[i] 
                        << " Re{z_coef}: " << cosPhi
                        << " Im{z_coef}: " << sinPhi);
            }
        }

        //timing
        initFrameReady_ = (delayLineSize_ - hopSize_) / blockSize;
        frameReady_ = hopSize_ / blockSize;
        maxCount_ = initFrameReady_;

        LOUDNESS_DEBUG(name_
                << ": Number of process calls until first frame: " 
                << initFrameReady_
                << "\n Number of process calls until subsequent frames: " 
                << frameReady_);

        delayWriteIdx_ = 0;
        count_ = 0;

        return 1;
    }

    void GoertzelPS::processInternal(const SignalBank &input)
    {
        //output trigger
        output_.setTrig(0);

        //fill delay line - array size is an integer multiple of the audio block size
        int nSamples = input.getNSamples();
        for(int i=0; i<nSamples; i++)
            delayLine_[delayWriteIdx_++] = input.getSample(0, i);
        delayWriteIdx_ = delayWriteIdx_ % delayLineSize_;
        count_++;
        
        //the Goertzels
        Real comb, v;
        for(int i=0; i<nWindows_; i++)
        {
            for(int k=0; k<nSamples; k++)
            {
                comb = delayLine_[startIdx_[i]++] - delayLine_[endIdx_[i]++];

                for(unsigned int j=0; j<vPrev_[i].size(); j++)
                {
                    v = comb + cosineTimes2_[i][j] * vPrev_[i][j] - vPrev2_[i][j];
                    vPrev2_[i][j] = vPrev_[i][j];			
                    vPrev_[i][j] = v;
                }

                startIdx_[i] = startIdx_[i] % delayLineSize_;
                endIdx_[i] = endIdx_[i] % delayLineSize_;
            }
        }

        if(count_ == maxCount_)
        {
            
            LOUDNESS_DEBUG(name_ << ": Computing new frame");
            LOUDNESS_DEBUG(name_ << ": write index: " << delayWriteIdx_);
            for(int i=0; i<nWindows_; i++)
                LOUDNESS_DEBUG(name_
                        << ": start index: " << startIdx_[i]
                        << ": end index: " << endIdx_[i]);

            /*
            #if defined(DEBUG)
            for(int i=0; i<nWindows_; i++)
            {
                for(unsigned int j=0; j<vPrev_[i].size(); j++)
                    LOUDNESS_DEBUG(name_ << ", v[n]: " << vPrev_[i][j]);
            }
            #endif
            */

            //window the spectrum and compute PS
            if(windowSpectrum_)
                windowedPS();
            else
                computePS();

            //output ready
            output_.setTrig(1);

            count_ = 0;
            maxCount_ = frameReady_;
        }
    }

    void GoertzelPS::computePS()
    {
        Real re, im;
        int binWriteIdx=0;
        for(int i=0; i<nWindows_; i++)
        {
            for(unsigned int j=1; j<(vPrev_[i].size()-1); j++)
            { 
                // r*cos(phi)*v[n]-v[n-1]
		re = (0.5*cosineTimes2_[i][j]
                        * vPrev_[i][j]) - vPrev2_[i][j];
		im = sine_[i][j]*vPrev_[i][j];	

                output_.setSample(binWriteIdx++, 0, norm_[i]*(re*re + im*im));
            }
        }
    }


    void GoertzelPS::windowedPS()
    {
	Real re, im, rePrev, imPrev, rePrevTemp, 
               imPrevTemp, rePrevTemp2, imPrevTemp2;
        int binWriteIdx=0;
        for(int i=0; i<nWindows_; i++)
        {
            for(unsigned int j=0; j<vPrev_[i].size(); j++)
            { 
                // r*cos(phi)*v[n]-v[n-1]
		re = (0.5*cosineTimes2_[i][j]
                        * vPrev_[i][j]) - vPrev2_[i][j];
		im = sine_[i][j]*vPrev_[i][j];	

                //checked out:17.7.14
		rePrevTemp2 = re;
		imPrevTemp2 = im;

		if (j>0)
		{
                    //real
                    rePrev -= re; //final bin not required
                    re *= 2.0;
                    re -= rePrevTemp;
                    //imag
                    imPrev -= im;
                    im *= 2.0;
                    im -= imPrevTemp;

                    //excludes redundant bins
                    if(j>1)
                        output_.setSample(binWriteIdx++, 0, norm_[i]*(rePrev*rePrev + imPrev*imPrev));
		}

		rePrev = re;
		imPrev = im;
		rePrevTemp = rePrevTemp2;
		imPrevTemp = imPrevTemp2;
            }
        }
    }

    void GoertzelPS::configureDelays()
    {
        startIdx_.resize(nWindows_);
        endIdx_.resize(nWindows_);
        delayLine_.assign(delayLineSize_, 0.0);
        delayWriteIdx_ = 0;

        for(int i=0; i<nWindows_; i++)
        {
            //SignalBank offset for temporal alignment
            Real tc2 = (windowSizeSamps_[i]-1)/2.0;
            if(windowSizeSamps_[i] == largestWindowSize_)
                startIdx_[i] = 0;
            else
                startIdx_[i] = delayLineSize_ - (int)round(temporalCentre_ - tc2);

            //check this line
            endIdx_[i] = (delayLineSize_ + startIdx_[i] - windowSizeSamps_[i]) % delayLineSize_;

            LOUDNESS_DEBUG(name_ 
                    << ": Window size: " << windowSizeSamps_[i]
                    << " centre point: " 
                    << ((startIdx_[i] + largestWindowSize_ ) % delayLineSize_) - tc2 - 1
                    << " start idx: " << startIdx_[i] 
                    << " end idx: " << endIdx_[i]);
        }
    }

    void GoertzelPS::resetInternal()
    {
        count_ = 0;
        maxCount_ = initFrameReady_;
        configureDelays();

        for(int i=0; i<nWindows_; i++)
        {
            for(unsigned int j=0; j<vPrev_[i].size(); j++)
                vPrev_[i][j] = vPrev2_[i][j] = 0.0;
        }
    }
}
