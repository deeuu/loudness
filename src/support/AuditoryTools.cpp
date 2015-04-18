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
 * Foobar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>. 
 */

#include "AuditoryTools.h"
#include "../thirdParty/spline/Spline.h"

namespace loudness{
    
    Real kdB(Real freq)
    {
        if (freq>=1000)
        {
            return -3.0;
        }
        else
        {
            Real logFreq;
            if(freq<=50)
                logFreq = log10(50);
            else
                logFreq = log10(freq);

            return 4.4683421395470813 * pow(logFreq,4) 
                - 49.680265975480935 * pow(logFreq,3) 
                + 212.89091871289099 * pow(logFreq,2) 
                - 418.88552055717832 * logFreq + 317.07466358701885;
        }
    };

    Real soneToPhonMGB1997(Real sone, bool isANSIS342007)
    {
	Real s = log(sone);

        if(s>=4.2536654906763038) //>100dB
        {
            if(isANSIS342007)
            {
                 return 0.15807691374474481*pow(s,4) - 3.3544564637191723*pow(s,3) +
                     26.263239744515747*pow(s,2) - 77.333716130360571*s + 160.22868135292651;
            }
            else
            {
                return 0.0011393249227292703*pow(s,4) + 0.13726126866961721*pow(s,3) -
                    3.7283448889230781*pow(s,2) + 35.999178091248588*s + 3.4328174085009824;
            }
        }
        else if(s>=-4.7137054648800527) //>5dB
        {
            return 0.00071270124863722598*pow(s,6) + 0.001706164242343532*pow(s,5) -
                0.036623964553686571*pow(s,4) - 0.12818464456821468*pow(s,3) +
                1.1644021388448895*pow(s,2) + 12.722826749265581*s + 40.018310769092793;
        }
        else if(s>=-6.7900957940165068) //>0dB
        {
            return 0.13705164343541046*pow(s,3) + 2.5683044970070505*pow(s,2) +
                18.190233135520316*s + 47.984354836906057;
        }
        else //<=0dB
        {
            return 0.00015209798556303997*pow(s,3) + 0.011105494476957356*pow(s,2) +
                1.9904308466721425*s + 12.839992137905567;
        }
    }
    
    OME::OME(const string& middleEarType, const string& outerEarType) :
        middleEarType_(middleEarType),
        outerEarType_(outerEarType)
    {
        LOUDNESS_DEBUG("OME: Constructed.");
    }

    void OME::getData()
    {
        //The data
        Real freqs[41] = {0, 20, 25, 31.5, 40, 50, 63, 80, 100, 125, 160, 200, 250,
            315, 400, 500, 630, 750, 800, 1000, 1250, 1500, 1600, 2000, 2500,
            3000, 3150, 4000, 5000, 6000, 6300, 8000, 9000, 10000, 11200, 12500,
            14000, 15000, 16000, 18000, 20000};
        
        Real freeANSI[41] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0.1, 0.3, 0.5, 0.9, 1.4,
                1.6, 1.7, 2.5, 2.7, 2.6, 2.6, 3.2, 5.2, 6.6, 12, 16.8, 15.3, 15.2,
                14.2, 10.7, 7.1, 6.4, 1.8, -0.9, -1.6, 1.9, 4.9, 2, -2, 2.5, 2.5, 2.5};

        Real diffuseANSI[41] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0.1, 0.3, 0.4, 0.5, 1,
            1.6, 1.7, 2.2, 2.7, 2.9, 3.8, 5.3, 6.8, 7.2, 10.2, 14.9, 14.5, 14.4, 
            12.7, 10.8, 8.9, 8.7, 8.5, 6.2, 5, 4.5, 4, 3.3, 2.6, 2, 2, 2};

        Real midANSI[41] = {-100, -39.6, -32, -25.85, -21.4, -18.5, -15.9, -14.1,
            -12.4, -11, -9.6, -8.3, -7.4, -6.2, -4.8, -3.8, -3.3, -2.9, 
            -2.6, -2.6, -4.5, -5.4, -6.1, -8.5,-10.4, -7.3, -7, -6.6,
            -7, -9.2, -10.2, -12.2, -10.8, -10.1, -12.7, -15, -18.2,
            -23.8, -32.3, -45.5, -50};

        //can get away with the same freq spec as ANSI standard
        Real midChenEtAl[41] = {-100, -33, -28.5, -23.6, -19.4, -16.4, -13.4,
            -10.3, -8, -6.3, -4.7, -3.7, -2.7, -2.5, -1.9, -1.8, -2.1, -2.4,
            -2.4, -2.6, -3.7, -5.5 , -6.8, -11.4, -14.4, -11.3, -11 , -10.5,
            -10.9, -12.9, -13.9, -16.4, -15.6, -15.1, -16.8, -18.9, -20.9,
            -21.8, -22.3, -23.2, -24};

        //Beyerdynamic DT990 requires own sample points
        Real phoneFreqs[86] = {0, 5, 11, 17, 23, 29, 35, 41, 46, 52, 58, 64, 70,
            76, 82, 88, 101, 116, 133, 153, 176, 202, 232, 266, 306, 352, 404,
            464, 533, 612, 703, 808, 928, 1066, 1224, 1406, 1615, 1856, 2131,
            2448, 2813, 3231, 3711, 3928, 4144, 4396, 4897, 5215, 5437, 5625,
            5780, 6110, 6500, 6800, 7422, 7920, 8400, 8526, 8830, 9000, 9160,
            9230, 9360, 9650, 9900, 10500, 10900, 11250, 11700, 12200, 12500,
            12923, 13500, 14000, 14350, 14600, 14750, 14845, 15350, 16100,
            16700, 17200, 17750, 18420, 19587, 20000};

        Real dt990[86] = {-6.45, -6.45, -7.48, -5.75, -3.69, -2.52, -1.61,
            -0.71, -0.42, 0.13,  0.24, -0.51,  0.56,  1.61,  2.02,  2.28,  2.6 ,
            2.79, 2.82,  2.69,  2.43,  2.09,  1.68,  1.31,  0.77,  0.15, -0.31,
            -0.64, -0.96, -1.36, -1.41, -1.16, -0.52,  0.  ,  1.24,  1.84, 2.8 ,
            4.28,  5.77,  7.5 ,  8.39,  7.2 ,  4.3 ,  3.37,  4.  , 4.52,  2.25,
            1.36,  2.26,  4.69,  7.01,  9.37,  9.6 ,  9.66, 7.8 ,  6.07,  8.25,
            8.9 ,  7.84,  5.42,  2.24,  0.73, -0.08, 0.69,  1.23,  2.01,  1.47,
            0.8 ,  0.54,  3.06,  3.5 ,  1.79, -3.52, -7.86, -8.68, -9.81, -7.1 ,
            -5.09, -1.14,  0.02,  1.6 , 0.25, -2.8 , -3.97, -7.45, -6.8 };
            
        //frequencies to use for middle ear interpolation
        middleEarFreqPoints_.assign(freqs, freqs + 41);

        //Middle ear
        if(middleEarType_ == "ANSIS342007")
            middleEardB_.assign(midANSI, midANSI + 41);
        else if(middleEarType_ == "ANSIS342007_HPF")
            middleEardB_.assign(midANSI, midANSI + 41);
        else if(middleEarType_ == "CHGM2011")
            middleEardB_.assign(midChenEtAl, midChenEtAl + 41);
        else
            middleEarType_ = "";

        //Outer ear
        if(outerEarType_ == "ANSIS342007_FREEFIELD")
        {
            outerEardB_.assign(freeANSI, freeANSI + 41);
            outerEarFreqPoints_ = middleEarFreqPoints_;
        }
        else if(outerEarType_ == "ANSIS342007_DIFFUSEFIELD")
        {
            outerEardB_.assign(diffuseANSI, diffuseANSI + 41);
            outerEarFreqPoints_ = middleEarFreqPoints_;
        }
        else if(outerEarType_ == "DT990")
        {
            outerEardB_.assign(dt990, dt990 + 86);
            outerEarFreqPoints_.assign(phoneFreqs, phoneFreqs + 86);
        }
        else
        {
            outerEarType_ = "";
        }
    }

    void OME::setMiddleEarType(const string& middleEarType)
    {
        middleEarType_ = middleEarType;
    }

    void OME::setOuterEarType(const string& outerEarType)
    {
        outerEarType_ = outerEarType;
    }
    bool OME::interpolateResponse(const RealVec &freqs)
    {
        if(freqs.empty())
        {
            LOUDNESS_ERROR("OME: Input vector has no elements.");
            return 0;
        }
        else
        {
            response_.resize(freqs.size(), 0.0);

            //load the data into vectors
            getData();    

            //the spline
            spline s;

            //middle ear
            if(!middleEarType_.empty())
            {
                s.set_points(middleEarFreqPoints_, middleEardB_);

                for (uint i=0; i < freqs.size(); i++)
                {
                    if((freqs[i]<=75) && (middleEarType_ == "ANSIS342007_HPF"))
                            response_[i] = -14.6; //Post HPF correction
                    else if(freqs[i] >= middleEarFreqPoints_[40])
                        response_[i] = middleEardB_[40];
                    else
                        response_[i] = s(freqs[i]);
                }
            }
            else
            {
                LOUDNESS_WARNING("OME: No middle ear filter used.");
            }

            //outer ear
            if(!outerEarType_.empty())
            {
                Real lastFreq = outerEarFreqPoints_.back();
                Real lastDataPoint = outerEardB_.back();
                s.set_points(outerEarFreqPoints_, outerEardB_);

                for(uint i = 0; i < freqs.size(); i++)
                {
                    if(freqs[i] >= lastFreq)
                        response_[i] += lastDataPoint;
                    else
                        response_[i] += s(freqs[i]);
                }
            }
            else
            {
                LOUDNESS_WARNING("OME: No outer ear filter used.");
            }

            return 1;
        }
    }

    const RealVec& OME::getResponse() const
    {
        return response_;
    }

    const RealVec& OME::getMiddleEardB() const
    {
        return middleEardB_;
    }

    const RealVec& OME::getOuterEardB() const
    {
        return outerEardB_;
    }

    const RealVec& OME::getOuterEarFreqPoints() const
    {
        return outerEarFreqPoints_;
    }

    const RealVec& OME::getMiddleEarFreqPoints() const
    {
        return middleEarFreqPoints_;
    }
}
