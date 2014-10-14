#include "RoexBankGM.h"
#include "../Support/AuditoryTools.h"

namespace loudness{

    RoexBankGM::RoexBankGM(Real camLo, Real camHi, Real camStep, bool approxRoex) :
        camLo_(camLo),
        camHi_(camHi),
        camStep_(camStep),
        approxRoex_(approxRoex)
    {
        LOUDNESS_DEBUG("RoexBankGM: Constructed.");
        name_ = "RoexBankGM";
    }

    RoexBankGM::~RoexBankGM()
    {
    }

    bool RoexBankGM::initializeInternal(const SignalBank &input)
    {

        /*
         * Level per ERB precalculations
         */
        rectBinIndices_.resize(input.getNChannels());

        for(int i=0; i<input.getNChannels(); i++)
        {
            //ERB number of Centre frequency
            Real cam = FreqToCam(input.getCentreFreq(i));

            //rectangular ERB band edges in Hz
            Real freqLo = CamToFreq(cam-0.5);
            Real freqHi = CamToFreq(cam+0.5);

            //lower and upper bin indices
            rectBinIndices_[i].resize(2);
            rectBinIndices_[i][0] = i;
            rectBinIndices_[i][1] = i+1;

            //sum components falling within the filter
            bool first = true;
            int j=0;
            while(j<input.getNChannels())
            {
                if(input.getCentreFreq(j)>freqHi) 
                    break;
                else if(input.getCentreFreq(j)>=freqLo)
                {
                    if(first)
                        rectBinIndices_[i][0] = j;
                    first = false;
                    rectBinIndices_[i][1] = j+1;
                }
                j++;
            }
        }	

        /*
         * Excitation pattern variables
         */

        //number of roex filters to use
        //+1 inclusive, can exceed end value if fractional part of 
        //argument to round is >=0.5
        nFilters_ = round((camHi_-camLo_)/camStep_)+1; 

        //initialize output SignalBank
        output_.initialize(nFilters_, 1, input.getFs());
        output_.setFrameRate(input.getFrameRate());

        const Real p51_1k = 4000 / FreqToERB(1000.0);

        //p upper is level invariant
        pu_.assign(nFilters_, 0.0);
        
        //p lower is level dependent
        pl_.assign(nFilters_, 0.0);

        //comp_level holds level per ERB on each component
        compLevel_.assign(input.getNChannels(), 0.0);

        //fill the above arrays and calculate roex filter response for upper skirt
        Real cam, erb, fc;
        for(int i=0; i<nFilters_; i++)
        {
            cam = camLo_+(i*camStep_);
            fc = CamToFreq(cam);
            erb = FreqToERB(fc);
            pu_[i] = 4*fc/erb;
            pl_[i] = 0.35*(pu_[i]/p51_1k);
            output_.setCentreFreq(i,fc);
        }

        if(approxRoex_)
            generateRoexTable(1024);
        
        return 1;
    }


    void RoexBankGM::processInternal(const SignalBank &input)
    {
        /*
         * Part 1: Obtain the level per ERB about each input component
         */
        Real runningSum = 0;
        int j = 0;
        int k = rectBinIndices_[0][0];
        int nChannels = input.getNChannels();
        for(int i=0; i<nChannels; i++)
        {
            //running sum of component powers
            while(j<rectBinIndices_[i][1])
                runningSum += input.getSample(j++,0);

            //subtract components outside the window
            while(k<rectBinIndices_[i][0])
                runningSum -= input.getSample(k++,0);

            //convert to dB, subtract 51 here to save operations later
            if (runningSum < 1e-10)
                compLevel_[i] = -151.0;
            else
                compLevel_[i] = 10*log10(runningSum)-51;

            LOUDNESS_DEBUG("RoexBankGM: ERB/dB : " << compLevel_[i]+1e-10);
        }

        /*
         * Part 2: Complete roex filter response and compute excitation per ERB
         */

        Real fc, g, p, pg, excitationLin;
        if(approxRoex_)
        {
            int idx;
            for(int i=0; i<nFilters_; i++)
            {
                excitationLin = 0;
                j = 0;
                fc = output_.getCentreFreq(i);

                while(j<nChannels)
                {
                    //normalised deviation
                    g = (input.getCentreFreq(j)-fc)/fc;

                    if(g>2)
                        break;
                    if(g<0) //lower skirt - level dependent
                    {
                        p = pu_[i]-(pl_[i]*compLevel_[j]); //51dB subtracted above
                        p = p < 0.1 ? 0.1 : p; //p can go negative for very high levels
                        pg = -p*g; 
                    }
                    else //upper skirt
                    {
                        pg = pu_[i]*g; 
                    }
                    
                    //excitation
                    idx = (int)(pg/step_ + 0.5);
                    idx = idx > roexIdxLimit_ ? roexIdxLimit_ : idx;
                    excitationLin += roexTable_[idx]*input.getSample(j++,0); 
                }

                //output
                output_.setSample(i, 0, excitationLin);
            }
        }
        else //duplicate code but safe some conditionals
        {
            for(int i=0; i<nFilters_; i++)
            {
                excitationLin = 0;
                j = 0;
                fc = output_.getCentreFreq(i);

                while(j<nChannels)
                {
                    //normalised deviation
                    g = (input.getCentreFreq(j)-fc)/fc;

                    if(g>2)
                        break;
                    if(g<0) //lower skirt - level dependent
                    {
                        p = pu_[i]-(pl_[i]*compLevel_[j]); //51dB subtracted above
                        p = p < 0.1 ? 0.1 : p; //lower limit needed for idx
                        pg = -p*g; 
                    }
                    else //upper skirt
                    {
                        pg = pu_[i]*g; 
                    }
                    
                    //excitation
                    excitationLin += (1+pg)*exp(-pg)*input.getSample(j++,0); 
                }

                //output
                output_.setSample(i, 0, excitationLin);
            }
        }
    }

    void RoexBankGM::resetInternal(){};

    void RoexBankGM::generateRoexTable(int size)
    {
        size = size < 512 ? 512 : size;
        roexIdxLimit_ = size-1;
        roexTable_.resize(size);

        double pgLim = 20.48; //end value is 20.46
        double pg;
        step_ = pgLim / size;
        
        for(int i=0; i<size; i++)
        {
            pg = step_*i;
            roexTable_[i] = (1+pg) * exp(-pg);
        }
    }
}

