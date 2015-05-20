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

#include "CompressSpectrum.h"
#include "../support/AuditoryTools.h"

namespace loudness{

    CompressSpectrum::CompressSpectrum(Real alpha) : 
        Module("CompressSpectrum"),
        alpha_(alpha)
    {}

    CompressSpectrum::~CompressSpectrum() {}

    bool CompressSpectrum::initializeInternal(const SignalBank &input)
    {
        LOUDNESS_ASSERT(input.getNChannels() > 1, name_ << ": Insufficient number of channels.");


        /*
         * This code is sloppy due to along time spent figuring how 
         * to implement the damn thing.
         * It's currently in two parts, one that searches for the limits of each
         * summation range in order to satisfy summation criterion.
         * The other that finds the average Centre frequencies per compressed band.
         */
        int nChannels = input.getNChannels();
        int i=0, binIdxPrev = 0;
        Real dif = freqToCam(input.getCentreFreq(1)) - 
                   freqToCam(input.getCentreFreq(0));
        int groupSize = max(2.0, std::floor(alpha_/(dif)));
        int groupSizePrev = groupSize;
        vector<int> groupSizeStore, binIdx;

        while(i < nChannels-1)
        {

            //compute different between adjacent bins on Cam scale 
            dif = freqToCam(input.getCentreFreq(i+1)) - freqToCam(input.getCentreFreq(i));

            //Check if we can sum bins in group size
            if(dif < (alpha_/double(groupSize)))
            {
                /*  
                 *  from here we can group bins in groupSize
                 *  whilst maintaining alpha spacing
                 */

                //Check we have zero idx
                if((binIdx.size() < 1) && (i>0))
                {
                    binIdx.push_back(0);
                    groupSizeStore.push_back(1);
                }

                /*
                 * This line ensures the next group starts at the next multiple of the previous
                 * groupSize above the previous starting position.
                 * This is why you sometimes get finer resolution than the criterion
                 */

                int store = ceil((i-binIdxPrev)/double(groupSizePrev))*groupSizePrev+binIdxPrev;
                
                /*  
                 *  This line is cheeky; it re-evaluates the groupSize at the new multiple
                 *  in attempt to maintain alpha spacing, I'm not 100% but the algorithm
                 *  seems to satisfy various criteria
                 */
                if((store > 0) && (store < nChannels))
                {
                    dif = freqToCam(input.getCentreFreq(store)) - 
                          freqToCam(input.getCentreFreq(store-1));
                    groupSize = max((double)groupSize, std::floor(alpha_/dif));
                }

                //fill variables
                groupSizePrev = groupSize;
                binIdxPrev = store;

                //storage
                binIdx.push_back(store);
                groupSizeStore.push_back(groupSize);
                //print "Bin: %d, Binnew: %d, composite bin size: %d" % (i, store, groupSize)

                //Move i along
                i = store+groupSize;

                //increment groupSize for wider group
                groupSize += 1;
            }
            else
                i += 1;
        }

        //add the final frequency
        if(binIdx[binIdx.size()-1] < nChannels)
            binIdx.push_back(nChannels);

        LOUDNESS_DEBUG(name_ << ": Hi Dom");

        //PART 2
        //compressed spectrum
        RealVec cfs;
        Real fa = 0;
        int count = 0;
        int j = 0;
        i = 0;
        while(i < nChannels)
        {
            //bounds check out?
            if(i<binIdx[j+1])
            {
                fa += input.getCentreFreq(i);
                count++;
                if (count==groupSizeStore[j])
                {
                    //upper limit
                    upperBandIdx_.push_back(i+1); //+1 for < conditional
                    //set the output frequency
                    cfs.push_back(fa/count);
                    count = 0;
                    fa = 0;
                }
                i++;
            }
            else
                j++;
        }

        //add the final component if it didn't make it
        if (count>0)
        {
            cfs.push_back(fa/count);
            upperBandIdx_.push_back(i); 
        }

        //check
        #if defined(DEBUG)
        Real freqLimit;
        for(unsigned int i=0; i<cfs.size()-1; i++)
        {
            if((freqToCam(cfs[i+1])-freqToCam(cfs[i])) > alpha_)
                freqLimit = cfs[i];
        }
        LOUDNESS_DEBUG("CompressSpectrum: Criterion satisfied above "
                << freqLimit << " Hz.");
        #endif

        //set output SignalBank
        output_.initialize(input.getNEars(), cfs.size(), 1, input.getFs());
        output_.setCentreFreqs(cfs);
        output_.setFrameRate(input.getFrameRate());
        LOUDNESS_DEBUG(name_ << ": Number of bins comprising the compressed spectrum: "
                << output_.getNChannels());

        return 1;
    }

    void CompressSpectrum::processInternal(const SignalBank &input)
    {
        for (int ear = 0; ear < input.getNEars(); ear++)
        {
            const Real* inputSpectrum = input.getSingleSampleReadPointer(ear, 0);
            Real* outputSpectrum = output_.getSingleSampleWritePointer(ear, 0);

            Real sum = 0.0;
            int i = 0, j = 0;
            while (i < output_.getNChannels()) 
            {
                if (j < upperBandIdx_[i])
                {
                    sum += inputSpectrum[j++];
                }
                else
                {
                    outputSpectrum[i++] = sum;
                    sum = 0.0;
                }
            }
        }
    }

    void CompressSpectrum::resetInternal(){};
}

