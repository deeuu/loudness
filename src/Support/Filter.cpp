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

#include "Filter.h"
#include "../cnpy/cnpy.h"

namespace loudness{

    Filter::Filter() : gain_(1.0)
    {}

    //Is this needed?
    Filter::Filter(int order) :
        gain_(1.0),
        order_(order)
    {}

    Filter::~Filter() {}

    bool Filter::loadCoefsFromNumpyArray(string pathToFilterCoefs)
    {
        if(pathToFilterCoefs.empty())
        {
           LOUDNESS_ERROR("Filter: No path to filter coefficients"); 
           return 0;
        }
        else
        {
            //load numpy array holding the filter coefficients
            //npy_load will abort if fopen fails
            cnpy::NpyArray arr = cnpy::npy_load(pathToFilterCoefs);
            Real *data = reinterpret_cast<Real*> (arr.data);

            //check if filter is IIR or FIR
            bool iir = false;
            if(arr.shape[0]==2)
            {
                iir = true;
            }
            else if(arr.shape[0]!=1)
            {
                LOUDNESS_ERROR("Filter: Numpy array shape should be 1xN (FIR) or 2xN (IIR).");
                return 0;
            }
            LOUDNESS_DEBUG("Filter: Shape is: (" << arr.shape[0] << " x " << arr.shape[1] << ")");

            //transfer data
            bCoefs_.assign(1,RealVecVec(1));
            aCoefs_.assign(1,RealVecVec(1));
            for(unsigned int i=0; i<arr.shape[1]; i++)
            {
                bCoefs_[0][0].push_back(data[i]);
                if(iir)
                    aCoefs_[0][0].push_back(data[i+arr.shape[1]]);
            }
            
            //clean up
            delete [] data;
        }

        return 1;
    }

    void Filter::setBCoefs(const RealVec &bCoefs)
    {
        bCoefs_.assign(1, RealVecVec(1, bCoefs));
    }

    void Filter::setACoefs(const RealVec &aCoefs)
    {
        aCoefs_.assign(1, RealVecVec(1, aCoefs));
    }

    void Filter::normaliseCoefs()
    {
        for(uint ear=0; ear<aCoefs_[0].size(); ear++)
        {
            for(uint chn=0; chn<aCoefs_[ear].size(); chn++)
            {
                if(aCoefs_[ear][chn].size()>0)
                {
                    if(aCoefs_[ear][chn][0]!=1.0)
                    {
                        LOUDNESS_DEBUG("Filter: Normalising filter coeffients.");
                        for(int i=(int)bCoefs_[ear][chn].size()-1; i>-1; i--)
                            bCoefs_[ear][chn][i] /= aCoefs_[ear][chn][0];
                        for(int i=(int)aCoefs_[ear][chn].size()-1; i>-1; i--)
                            aCoefs_[ear][chn][i] /= aCoefs_[ear][chn][0];
                    }
                    else
                        LOUDNESS_WARNING("Filter: Coefficient a[0] == 0.");
                }
            }
        }
    }

    void Filter::resetDelayLine()
    {
        //zero delay line
        for(uint ear=0; ear<z_.size(); ear++)
            for(uint chn=0; chn<z_[ear].size(); chn++)
                for(uint i=0; i<z_[ear][chn].size(); i++)
                    z_[ear][chn][i] = 0.0;
    }
    
    bool Filter::checkBCoefs(const SignalBank& input)
    {
        //check filter configuration
        if(input.getNEars() == (int)bCoefs_.size())
        {
            duplicateEarCoefs_ = false;
        }
        else if((input.getNEars()>1) && (bCoefs_.size()==1))
        {
            duplicateEarCoefs_ = true;
        }
        else
        {
            LOUDNESS_WARNING("Filter: Either single vector of filter coefficients or one per ear.");
            return 0;
        }

        return 1;
    }

    const RealVec &Filter::getBCoefs() const
    {
        return bCoefs_[0][0];
    }

    const RealVec &Filter::getACoefs() const
    {
        return aCoefs_[0][0];
    }

    Real Filter::getGain() const
    {
        return gain_;
    }

    int Filter::getOrder() const
    {
        return order_;
    }

    void Filter::setGain(Real gain)
    {
        gain_ = gain;
    }
}
