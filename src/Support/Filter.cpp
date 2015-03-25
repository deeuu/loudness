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
            for(unsigned int i=0; i<arr.shape[1]; i++)
            {
                bCoefs_.push_back(data[i]);
                if(iir)
                    aCoefs_.push_back(data[i+arr.shape[1]]);
            }
            
            //clean up
            delete [] data;
        }

        return 1;
    }

    void Filter::setBCoefs(const RealVec &bCoefs)
    {
        bCoefs_ = bCoefs;
    }

    void Filter::setACoefs(const RealVec &aCoefs)
    {
        aCoefs_ = aCoefs;
    }

    void Filter::normaliseCoefs()
    {
        if (aCoefs_.size() > 0)
        {
            if (aCoefs_[0] != 1)
            {
                LOUDNESS_DEBUG("Filter: Normalising filter coeffients.");
                for(int i=(int)bCoefs_.size()-1; i>-1; i--)
                    bCoefs_[i] /= aCoefs_[0];
                for(int i=(int)aCoefs_.size()-1; i>-1; i--)
                    aCoefs_[i] /= aCoefs_[0];
            }
            else
                LOUDNESS_DEBUG("Filter: Coefficient a[0] == 1, normalisation not required.");
        }
    }

    void Filter::resetDelayLine()
    {
        //zero delay line
        z_.assign(z_.size(), 0.0);
    }
    
    const RealVec &Filter::getBCoefs() const
    {
        return bCoefs_;
    }

    const RealVec &Filter::getACoefs() const
    {
        return aCoefs_;
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
