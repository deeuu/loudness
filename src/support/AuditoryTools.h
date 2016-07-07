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
     * @brief Returns the ERB in Cams of the auditory filter for normally hearing
     * listeners at the @a centreFreq in Hz.
     *
     * See ANSI S3.04-2007 sec 3.5 Eq (1)
     *
     * @return ERB in Cams.
     */
    inline Real centreFreqToCambridgeERB(Real centreFreq)
    {
        return 24.673 * (4368e-6 * centreFreq + 1.0);
    }

    /**
     * @brief Converts frequency in Hz to corresponding ERB number in Cams.
     *
     * See ANSI S3.04-2007 sec 3.5 Eq (4)
     *
     * @param freq Frequency in Hz.
     *
     * @return ERB number in Cams.
     */
    inline Real hertzToCam (Real freq)
    {
        return 21.366 * log10(4368e-6 * freq + 1.0);
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
    inline Real camToHertz(Real cam)
    {
        return (pow (10.0, (cam / 21.366)) - 1.0) / 4368e-6;
    }

    /**
     * @brief Returns the ERB in Bark of the auditory filter for normally hearing
     * listeners at @a centreFreq in Hz.
     *
     * Hartmann, 1997 p.249. Signals, Sound and Sensation.
     *
     * @return ERB in Bark.
     */
    inline Real centreFreqToMunichERB(Real centreFreq)
    {
        return 25.0 + 75.0 * pow(1.0 + 1.4e-6 * centreFreq*centreFreq, 0.69);
    }

    /**
     * @brief Converts frequency in Hz to corresponding critical-band number in Bark.
     *
     * Hartmann, 1997 p.252. Signals, Sound and Sensation.
     *
     * @param freq Frequency in Hz.
     *
     * @return Critical-band number in Bark;
     */
    inline Real hertzToBark (Real freq)
    {
        Real z = 26.81 * freq / (1960 + freq) - 0.53;
        if (z < 2.0)
            z = z + 0.15 * (2.0 - z);
        else if (z > 20.1)
            z = z + 0.22 * (z - 20.1);
        return z;
    }

    /**
     * @brief Converts critical-band number in Bark to frequency in Hz.
     *
     * Hartmann, 1997 p.253. Signals, Sound and Sensation.
     *
     * @param z Critical-band number in Bark.
     *
     * @return Frequency in Hz.
     */
    inline Real barkToHertz(Real z)
    {
        if (z < 2.0)
            z = 2.0 + (z - 2.0) / 0.85;
        else if (z > 20.1)
            z = 20.1 + (z - 20.1) / 1.22;
        return 1960 * (z + 0.53) / (26.28 - z);
    }

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
     * Piecewise polynomials were calculating by inputting a 1kHz tone at levels
     * -50-137 dB SPL using the RoexBankANSIS342007 module with filters spanning
     * 1.8 - 38.9 Cams with 0.1 Cam spacing. If you want the conversion according
     * to the revised specific loudness formula at high levels, set @a
     * isANSIS342007 to true. Functions are accurate to within +/- 0.43 phons
     * across the entire range and +/- 0.032 between 2 and 2.5 phons (useful
     * for predicting absolute thresholds) for both equations.
     *
     * @param sone Loudness in sones.
     *  
     * @param isANSIS3407 High level equation according to ANSI S34 2007 (true)
     * or Moore et al. 1997 paper (false).
     *
     * @return Loudness in phons.
     */
    Real soneToPhonMGB1997(Real sone, bool isANSIS342007);

    /**
     * @brief Returns loudness level in phons given loudness in sones.  
     *
     * Piecewise polynomials were calculating by inputting a 1kHz tone at levels
     * -50-137 dB SPL using the DoubleRoexBank module with filters spanning
     * 1.5 - 40.2 Cams with a 0.1 Cam spacing. Values are accurate to within +/- 0.41 phons
     * across the entire range and +/- 0.005 between 1.5 and 2.5 phons (useful
     * for predicting absolute thresholds).
     *
     * @param sone Loudness in sones.
     *  
     * @return Loudness in phons.
     */
    Real soneToPhonCHGM2011 (Real sone);

    /**
     * @brief Returns loudness level in phons given loudness in sones.
     *
     * Piecewise polynomials were calculating by inputting a 1kHz tone at levels
     * spanning 5 - 120 dB SPL using the model StationaryLoudnessDIN456311991 with
     * isOutputRounded_ set to false.  Values are accurate to within +/- 0.35
     * phons across the entire range of input levels tested.  
     *
     * If @a isPolyApprox is false (default is true), the transformation is
     * calculated using the equations given in Zwicker et al. (1991). However,
     * these functions appears to be less accurate than the polynomial
     * approximations (at least for the implementation of the model presented in
     * this library).
     *
     * REFERENCES:
     *
     * Zwicker, E., Fastl, H., Widmann, U., Kurakata, K., Kuwano, S., & Germtuo,
     * E. R. (1991). Program for calculating loudness according to DIN 45631 (ISO
     * 532B). Journal of the Acoustical Society of America, 12(1), 39–42.
     */
    Real soneToPhonDIN456311991 (Real sone, bool isPolyApprox = true);

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
     * @todo Add Sennheiser HD600 response
     */

    class OME{
        public:

            enum Filter{
                NONE,
                CHGM2011_MIDDLE_EAR,
                ANSIS342007_MIDDLE_EAR,
                ANSIS342007_MIDDLE_EAR_HPF,
                WARD_MIDDLE_EAR,
                ANSIS342007_FREEFIELD,
                ANSIS342007_DIFFUSEFIELD,
                BD_DT990
            };

            OME(){};
            OME(const Filter& middleEarType, const Filter& outerEarType);
            ~OME() {};

            void setMiddleEarType(const Filter& middleEarType);
            void setOuterEarType(const Filter& outerEarType);
            bool interpolateResponse(const RealVec &freqs);
            const RealVec& getResponse() const;
            const RealVec& getMiddleEardB() const;
            const RealVec& getOuterEardB() const;
            const RealVec& getMiddleEarFreqPoints() const;
            const RealVec& getOuterEarFreqPoints() const;

        private:
            void getData();
            Filter middleEarType_, outerEarType_;
            RealVec middleEarFreqPoints_, outerEarFreqPoints_;
            RealVec middleEardB_, outerEardB_, response_;
    };
}

#endif
