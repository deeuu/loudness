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

#ifndef AUDITORYTOOLS_H
#define AUDITORYTOOLS_H

#include "Common.h"

namespace loudness{

    /**
     * @brief Returns the ERB of the auditory filter for normally hearing
     * listeners at centre frequency @a freq in Hz.
     *
     * See ANSI S3.04-2007 sec 3.5
     *
     * @param freq Centre frequency in Hz.
     *
     * @return ERB.
     */
    inline Real freqToERB(Real freq)
    {
        return 24.673*(4368e-6*freq+1);
    }

    /**
     * @brief Converts frequency in Hz to corresponding ERB number in Cams.
     *
     * See ANSI S3.04-2007 sec 3.5
     *
     * @param freq Frequency in Hz.
     *
     * @return ERB number in Cams.
     */
    inline Real freqToCam(Real freq)
    {
        return 21.366*log10(4368e-6*freq+1);
    }

    /**
     * @brief Converts ERB number in Cams to frequency in Hz.
     *
     * See ANSI S3.04-2007 sec 3.5
     *
     * @param cam ERB number in Cams.
     *
     * @return Frequency in Hz.
     */
    inline Real camToFreq(Real cam)
    {
        return (pow(10, (cam/21.366))-1)/4368e-6;
    }

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

    /**
     * @brief Returns the detection efficiency parameter K (in decibels) corresponding
     * to the input frequency @freq in Hz.
     *
     * This is a fourth order polynomial fit to data points obtained from visual
     * inspection of the Figure 9 in Moore et al (1997).
     */
    Real kdB(Real freq);
    /**
     * @brief Returns loudness level in phons given loudness in sones.  
     *
     * Values were calculating by inputting a 1kHz tone at
     * levels -50-137 dB SPL using the RoexBankANSI07 module with filters
     * spanning 50-15000 Hz with 0.1 Cam spacing.     
     * Functions are accurate within +/- 0.22 phon across the entire range and
     * +/-0.021 between 2 and 2.5 phons (useful for predicting absolute thresholds).
     *
     * @param sone Loudness in sones.
     * @param ansiS3407 High level equation according to ANSI S34 2007 (true)
     * or AES 1997 paper (false).
     *
     * @return Loudness in phons.
     */
    Real soneToPhon(Real sone, bool ansiS3407);

    /**
     * @class OME
     *
     * @brief Returns interpolated transmission response of the outer and middle
     * ear.
     *
     * Three middle ear functions are available:
     * 1. ANSI S3.4 2007
     * 2. ANSI S3.4 2007 with mods for combination with third order 50Hz high pass filter
     * 3. Chen et al 2011
     *
     * The values for Chen et al 2011 were derived from visual inspection and
     * give a good fit using cubic interpolation.
     *
     * Two outer ear functions are available:
     * 1. Free-field to ear drum (frontal-incedence)
     * 2. Diffuse-field to ear drum (frontal-incedence)
     *
     * @todo Add BeyerDynamic DT990 response
     * @todo Add Sennheiser HD600 response
     *
     * @return 
     */

    class OME{
        public:

            OME(){};
            OME(const string& middleEarType, const string& outerEarType);
            ~OME() {};

            void setMiddleEarType(const string& middleEarType);
            void setOuterEarType(const string& outerEarType);
            //This should really take a RealVec by reference and modify
            //However, I need to get non-const vectors working with swig
            bool interpolateResponse(const RealVec &freqs);
            const RealVec& getResponse() const;
            const RealVec& getFreqPoints() const;
            const RealVec& getMiddleEardB() const;
            const RealVec& getOuterEardB() const;

        private:
            void getData();
            string middleEarType_, outerEarType_;
            RealVec freqPoints_, phoneFreqPoints_, middleEardB_, outerEardB_, response_;
            bool usingPhones_;
    };
}

#endif
