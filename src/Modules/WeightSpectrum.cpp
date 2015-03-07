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

#include "WeightSpectrum.h"

namespace loudness{

    WeightSpectrum::WeightSpectrum(const RealVec &weights) :
        Module("WeightSpectrum"),
        weights_(weights),
        usingOME_(false)
    {}

    WeightSpectrum::WeightSpectrum(const string& middleEarType, const string& outerEarType) :
        Module("WeightSpectrum"),
        ome_(middleEarType, outerEarType),
        usingOME_(true)
    {}

    WeightSpectrum::~WeightSpectrum() {}

    bool WeightSpectrum::initializeInternal(const SignalBank &input)
    {
        if(usingOME_)
        {
            ome_.interpolateResponse(input.getCentreFreqs());
            weights_ = ome_.getResponse();
        }

        if((int)weights_.size() != input.getNChannels())
        {
            LOUDNESS_WARNING(name_
                    << ": weights vector size is not equal to number of input channels,"
                    << " setting vector to unity gain");
            weights_.assign(input.getNChannels(), 1.0);
        }
        
        //set output SignalBank
        output_.initialize(input);

        //convert to linear power units for weighting power spectrum
        for(int i=0; i<output_.getNChannels(); i++)
            weights_[i] = pow(10, weights_[i]/10.0);

        return 1;
    }

    void WeightSpectrum::processInternal(const SignalBank &input)
    {
        for(int i=0; i<input.getNChannels(); i++)
            output_.setSample(i, 0, input.getSample(i,0)*weights_[i]);
    }

    void WeightSpectrum::setWeights(const RealVec &weights)
    {
        weights_ = weights;
    }

    void WeightSpectrum::resetInternal(){};
}

