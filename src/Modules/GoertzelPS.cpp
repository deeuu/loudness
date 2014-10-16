#include "GoertzelPS.h"

namespace loudness{
    
    GoertzelPS::GoertzelPS(const RealVec& bandFreqsHz,
            const RealVec& windowSizeSecs,
            Real hopSizeSecs) :
            Module("GoertzelPS"),
            bandFreqsHz_(bandFreqsHz),
            windowSizeSecs_(windowSizeSecs),
            hopSizeSecs_(hopSizeSecs)
    {}

    GoertzelPS::~GoertzelPS() {}

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
        int largestWindowSize_=0;
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
        hop_ = round(fs*hopSizeSecs_);
        blockSize_ = input.getNSamples();
        if(hop_<blockSize_)
        {
            LOUDNESS_DEBUG(name_
                    << ": Hop size is less than "
                    << "input buffer size, automatically correcting...");

        }
        else if(0!=(hop_%blockSize_))
        {
            LOUDNESS_DEBUG(name_
                    << ": Hop size is not a multiple of input "
                    << "buffer size, automatically correcting...");
        }
        hop_ = blockSize_*ceil(hop_/(Real)blockSize_);
        LOUDNESS_DEBUG(name_
                << ": Hop size in samples: " << hop_);

        //initialize the delay line make integer multiple of blockSize_
        delayLineSize_ = blockSize_*ceil(largestWindowSize_/(Real)blockSize_) + hop_;
        LOUDNESS_DEBUG(name_
                << ": Delay line size: "
                << delayLineSize_);
        
        delayLine_.assign(delayLineSize_, 0.0);

        //delay line read position per frame
        delayWriteIdx_ = 0;

        //binLimits contains the desired bins indices (lo and hi) per band
        vector<vector<int> > bandBinIndices(nWindows_);

        //appropriate delay for temporal alignment
        temporalCentre_ = (largestWindowSize_-1)/2.0;

        LOUDNESS_DEBUG(name_
                << ": Temporal centre of largest window: " 
                << temporalCentre_);

        startIdx_.resize(nWindows_);
        endIdx_.resize(nWindows_);
        norm_.resize(nWindows_);
    
        
         


        configureDelays();
        for(int i=0; i<nWindows_; i++)
        {
            //SignalBank offset for temporal alignment
            int tc2 = (windowSizeSamps_[i]-1)/2.0;
            if(windowSizeSamps_[i] == largestWindowSize_)
                startIdx_[i] = 0;
            else
                startIdx_[i] = delayLineSize_ - (int)round(temporalCentre_ - tc2);
            endIdx_[i] = (startIdx_[i] + windowSizeSamps_[i]) % largestWindowSize_;

            LOUDNESS_DEBUG(name_ 
                    << ": Window size: " windowSizeSamps_[i]
                    << " temporal centre: "
                    << tc2
                    << " start idx: " 
                    << startIdx_[i] 
                    << " end idx: " 
                    << endIdx_[i]);
            
            //bin indices to use for compiled spectrum
            bandBinIndices[i].resize(2);
            //These are NOT the nearest components but satisfies f_k in [f_lo, f_hi)
            bandBinIndices[i][0] = ceil(bandFreqsHz_[i]*windowSizeSamps_[i]/fs);
            bandBinIndices[i][1] = ceil(bandFreqsHz_[i+1]*windowSizeSamps_[i]/fs)-1;

            //exclude DC and Nyquist if found
            if(bandBinIndices[i][0]==0)
            {
                LOUDNESS_WARNING(name_ << " : DC found...excluding.");
                bandBinIndices[i][0] = 1;
            }
            if(bandBinIndices[i][1] >= (windowSizeSamps_[i]/2.0))
            {
                LOUDNESS_WARNING(name_ ": Bin is >= nyquist...excluding.");
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
            Real edgeLo = bandBinIndices[i][0]*fs/(float)windowSizeSamps_[i];
            Real edgeHi = bandBinIndices[i][1]*fs/(float)windowSizeSamps_[i];
            LOUDNESS_DEBUG(name_ 
                    << ": Band edges (Hz) for Window of size: " 
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
        output_.setFrameRate(fs/(Real)hop_);

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
		sine_[i].push_back(sin(2*PI*j/(Real)windowSizeSamps_[i]));
                cosineTimes2_[i].push_back(2*cos(2*PI*j/(Real)windowSizeSamps_[i])); 
                vPrev_[i].push_back(0.0);
                vPrev2_[i].push_back(0.0);

                LOUDNESS_DEBUG(name_ 
                        << ": Freq: " << j*fs/(Real)windowSizeSamps_[i] 
                        << " Re{z_coef}: " << 0.5*cosineTimes2_[i][k] 
                        << " Im{z_coef}: " << sine_[i][k]);
            }
        }

        //frame timing variables...first frame ready at end of block
        ready_ = blockSize_*ceil(largestWindowSize_/(Real)blockSize_);
        LOUDNESS_DEBUG(name_
                << ": First frame ready after receiving " << ready_ << " samples.");

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
              
        //the Goertzels
        int tempIdx1, tempIdx2;
        Real comb, v;
        for(int i=0; i<nWindows_; i++)
        {
            tempIdx1 = startIdx_[i];
            tempIdx2 = endIdx_[i];

            for(int k=0; k<nSamples; k++)
            {
                comb = delayLine_[tempIdx1++] - delayLine_[tempIdx2++];

                for(unsigned int j=0; j<vPrev_[i].size(); j++)
                {
                    v = comb + cosineTimes2_[i][j] * vPrev_[i][j] - vPrev2_[i][j];
                    vPrev2_[i][j] = vPrev_[i][j];			
                    vPrev_[i][j] = v;
                }

                tempIdx1 = tempIdx1 % delayLineSize_;
                tempIdx2 = tempIdx2 % delayLineSize_;
            }

            startIdx_[i] = tempIdx1;
            endIdx_[i] = tempIdx2;
        }

        if (delayWriteIdx_ == ready_)
        {
            delayWriteIdx_  = delayWriteIdx_ % delayLineSize_;
            ready_ = delayWriteIdx_+hop_;

            //window the spectrum and compute PS
            windowedPS();

            //output ready
            output_.setTrig(1);
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
		re = (0.5*cosineTimes2_[i][j]
                        * vPrev_[i][j]) - vPrev2_[i][j]; // cos(phi)*v[n]-v[n-1]
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

    void configureDelays()
    {
        for(int i=0; i<nWindows_; i++)
        {
            //SignalBank offset for temporal alignment
            int tc2 = (windowSizeSamps_[i]-1)/2.0;
            if(windowSizeSamps_[i] == largestWindowSize_)
                startIdx_[i] = 0;
            else
                startIdx_[i] = delayLineSize_ - (int)round(temporalCentre_ - tc2);
            endIdx_[i] = (startIdx_[i] + windowSizeSamps_[i]) % largestWindowSize_;

            LOUDNESS_DEBUG(name_ 
                    << ": Window size: " windowSizeSamps_[i]
                    << " temporal centre: "
                    << tc2
                    << " start idx: " 
                    << startIdx_[i] 
                    << " end idx: " 
                    << endIdx_[i]);
        }
    }

    void GoertzelPS::resetInternal()
    {
        delayWriteIdx_ = 0;
        ready_ = blockSize_*ceil(windowSizeSamps_[0]/(Real)blockSize_);

        delayLine_.assign(delayLineSize_, 0.0);

        configureDelays();

        ready_ = blockSize_*ceil(largestWindowSize_/(Real)blockSize_);

        for(int i=0; i<nWindows_; i++)
        {
            for(unsigned int j=0; j<vPrev_[i].size(); j++)
                vPrev_[i][j] = vPrev2_[i][j] = 0.0;
        }
    }
}
