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

#ifndef FILTER_H
#define FILTER_H

#include "../Support/Module.h"

namespace loudness{

    /**
     * @class Filter
     *
     * @brief Abstract class for other filter subclasses.
     *
     * This class provides some general functions common to digital filters.
     *
     * @author Dominic Ward
     *
     * @sa FIR, IIR, Butter
     */
    class Filter
    {
    public:
        Filter();
        Filter(int order);
        virtual ~Filter();

        /**
         * @brief Loads feedforward (@a bCoefs) and/or feedback (@a aCofs)
         * filter coefficients from a Numpy array binary file.
         * 
         * The shape of the array should be (1xN) and (2xN) for FIR and IIR
         * filters respectively, where N is the filter order plus one.
         *
         * I have not implemented this method as a constructor as the loading of
         * a Numpy array can fail.
         *
         * @param pathToFilterCoefs A string representing the path to the Numpy
         * array binary file. The file name must end with the
         * .npy extension.
         *
         * @return true if successfully loaded, false otherwise.
         */
        bool loadCoefsFromNumpyArray(string pathToFilterCoefs="");

        /**
         * @brief Sets the feedforward coefficients.
         */
        void setBCoefs(const RealVec &bCoefs);

        /**
         * @brief Sets the feedback coefficients.
         */
        void setACoefs(const RealVec &aCoefs);

        /**
         * @brief Normalises the filter coefficients by the first feedback
         * coefficient (aCoefs[0]);
         */
        void normaliseCoefs();

        const RealVec& getBCoefs() const;
        const RealVec& getACoefs() const;
         
        /**
         * @brief Sets the linear gain of the digital filter.
         */
        void setGain(Real gain);

        /**
         * @brief Returns the linear gain of the digital filter.
         */
        Real getGain() const;

        /**
         * @brief Returns the order of the digital filter.
         *
         * This is equal to the maximum number of feedforward
         * or feedback coefficients minus one.
         */
        int getOrder() const;

        /**
         * @brief Clears the internal state of the filter.
         */
        void resetDelayLine();

    protected:
        Real gain_;
        int order_, orderMinus1_;
        RealVec bCoefs_, aCoefs_, z_;
        bool duplicateEarCoefs_;
    };
}

#endif
