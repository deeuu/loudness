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

#ifndef SpecificPartialLoudnessMGB1997_H
#define SpecificPartialLoudnessMGB1997_H

#include "../support/Module.h"

namespace loudness{

    /**
     * @class SpecificPartialLoudnessMGB1997
     *
     * @brief Applies a piecewise function to an input excitation pattern (in
     * linear power units) and outputs a corresponding specific loudness
     * pattern.
     *
     * Models the compressive behaviour of the active cochlea mechanism
     * according to:
     * 
     * Moore, B. C. J., Glasberg, B. R., & Baer, T. (1997).  A Model for the
     * Prediction of Thresholds, Loudness, and Partial Loudness.  Journal of the
     * Audio Engineering Society, 45(4), 224–240.
     *
     * ANSI. (2007). ANSI S3.4-2007. Procedure for the Computation of Loudness
     * of Steady Sounds.
     *
     * Note that the specific loudness parameters are approximated using
     * polynomials rather than interpolted values as used by the ANSI S3.4 2007
     * standard.
     */
    class SpecificPartialLoudnessMGB1997 : public Module
    {
    public:

        SpecificPartialLoudnessMGB1997(bool useANSISpecificLoudness = true,
                bool updateParameterCForBinauralInhibition_ = false);

        /**
         * @brief Returns the excitation level (dB) at threshold at the
         * centre frequency @a freq in Hz.
         * 
         * This is a third order polynomial fit to the data given in Table 4 ANSI
         * S3.04-2007.
         */
        Real internalExcitation(Real freq);

        /**
         * @brief Returns the value of parameter A given G in dB.
         *
         * This is a fifth order polynomial fit to the data given in Table 6 ANSI
         * S3.04-2007.
         */
        Real gdBToA(Real gdB);

        /**
         * @brief Returns the value of parameter alpha given G in dB.
         *
         * This is a second order polynomial fit to the data given in Table 5 ANSI
         * S3.04-2007.
         */
        Real gdBToAlpha(Real gdB);

        /** Set parameter C as used for scaling the specific loudness values.
         */
        void setParameterC(Real parameterC);

        /**
         * @brief Returns the detection efficiency parameter K (in decibels) corresponding
         * to the input frequency @freq in Hz.
         *
         * This is a fourth order polynomial fit to data points obtained from visual
         * inspection of the Figure 9 in Moore et al (1997).
         */
        Real kdB(Real freq);

        virtual ~SpecificPartialLoudnessMGB1997();

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual bool initializeInternal(){return 0;};
        virtual void processInternal(const SignalBank &input);
        virtual void processInternal(){};
        virtual void resetInternal();

        bool useANSISpecificLoudness_, updateParameterCForBinauralInhibition_;
        Real parameterC_, parameterC2_, yearExp_;
        RealVec eThrqParam_, gParam_, aParam_, alphaParam_, kParam_;
    };
}

#endif
