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

#ifndef WEIGHTSPECTRUM_H
#define WEIGHTSPECTRUM_H

#include "../support/Module.h"
#include "../support/AuditoryTools.h"

namespace loudness{

    /**
     * @class WeightSpectrum
     *
     * @brief Filters an input SignalBank by applying a set of weights.
     *
     * This algorithm scales an input SignalBank using the set of weights. The
     * weights should be specified in decibels. As the name implies, this module
     * is used for modifying a short-time power spectrum, such as the kind
     * output by PowerSpectrum.
     *
     */
    class WeightSpectrum : public Module
    {

    public:

        /**
         * @brief Constructs the module using a set of input weights.
         *
         * @param vector of weights (in decibels).
         */
        WeightSpectrum(const RealVec &weights);

        /**
         * @brief Constructs the module for use with class OME.
         *
         * @param middleEarType The middle ear type.
         * @param outerEarType The outer ear type.
         */
        WeightSpectrum(const string& middleEarType,const string& outerEarType);

        virtual ~WeightSpectrum();

        /**
         * @brief Set the vector of weights (in decibels).
         */
        void setWeights(const RealVec &weights);

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();

        RealVec weights_;
        OME ome_;
        bool usingOME_;
    };
}

#endif
