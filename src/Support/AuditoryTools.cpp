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
#include "Spline.h"

namespace loudness{

    Real IntExc(Real freq)
    {
        if (freq>=500)
        {
            return 3.73;
        }
        else
        {
            Real logFreq;
            if(freq<=50)
                logFreq = log10(50);
            else
                logFreq = log10(freq);

            return pow(10, (0.43068810954936998 * pow(logFreq,3) 
                        - 2.7976098820730675 * pow(logFreq,2) 
                        + 5.0738460335696969 * logFreq -
                        1.2060617476790148));
        }
    };

    Real GdBToA(Real gdB)
    {
        if(gdB>=0)
        {
            return 4.72096;
        }
        else
        {
            return -0.0000010706497192096045 * pow(gdB,5) 
                -0.000060648487122230512 * pow(gdB,4) 
                -0.0012047326575717733 * pow(gdB,3) 
                -0.0068190417911848525 * pow(gdB,2)
                -0.11847825641628305 * gdB
                + 4.7138722463497347;
        }
    }

    Real GdBToAlpha(Real gdB)
    {
        if(gdB>=0)
        {
            return 0.2;
        }
        else
        {
            return 0.000026864285714285498 * pow(gdB,2)
                -0.0020023357142857231 * gdB + 0.19993107142857139;
        }
    }

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

    Real SoneToPhon(Real sone, bool ansiS3407)
    {
	Real s = log(sone);

        if(s>=4.2536654906763038) //>100dB
        {
            if(ansiS3407)
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
    
    OME::OME(MiddleEarType middleEarType, OuterEarType outerEarType) :
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

        Real midChenEtAl[41] = {-100, -33, -28.5, -23.6, -19.4, -16.4, -13.4,
            -10.3, -8, -6.3, -4.7, -3.7, -2.7, -2.5, -1.9, -1.8, -2.1, -2.4,
            -2.4, -2.6, -3.7, -5.5 , -6.8, -11.4, -14.4, -11.3, -11 , -10.5,
            -10.9, -12.9, -13.9, -16.4, -15.6, -15.1, -16.8, -18.9, -20.9,
            -21.8, -22.3, -23.2, -24};

        //put data into vectors
        freqVec_.assign(freqs, freqs + 41);

        switch(middleEarType_)
        {
            case ANSI:
            {
                middleEardB_.assign(midANSI, midANSI + 41);
                break;
            }
            case ANSI_HPF:
            {
                middleEardB_.assign(midANSI, midANSI + 41);
                break;
            }
            case CHEN_ETAL:
            {
                middleEardB_.assign(midChenEtAl, midChenEtAl + 41);
                break;
            }
            default:
            {
                middleEardB_.assign(midANSI, midANSI + 41);
                middleEarType_ = ANSI;
            }
        }
        switch(outerEarType_)
        {
            case ANSI_FREE:
            {
                outerEardB_.assign(freeANSI, freeANSI + 41);
                break;
            }
            case ANSI_DIFFUSE:
            {
                outerEardB_.assign(diffuseANSI, diffuseANSI + 41);
                break;
            }
            default:
            {
                outerEardB_.assign(freeANSI, freeANSI + 41);
                outerEarType_ = ANSI_FREE;
            }
        }
    }

    void OME::setMiddleEarType(MiddleEarType middleEarType)
    {
        middleEarType_ = middleEarType;
    }

    void OME::setOuterEarType(OuterEarType outerEarType)
    {
        outerEarType_ = outerEarType;
    }
    bool OME::interpolateResponse(const RealVec &freqs, RealVec &response)
    {
        if(freqs.size() != response.size())
        {
            LOUDNESS_ERROR("OME: Input vectors (freqs and response) are not equal");
            return 0;
        }
        else
        {
            //load the data into vectors
            getData();    

            //the spline
            spline s;

            //middle ear
            s.set_points(freqVec_, middleEardB_);
            for(unsigned int i=0; i<freqs.size(); i++)
            {
                if((freqs[i]<=75) && (middleEarType_ == ANSI_HPF))
                        response[i] = -14.6; //Post HPF correction
                else if(freqs[i]>=freqVec_[40])
                    response[i] = middleEardB_[40];
                else
                    response[i] = s(freqs[i]);
            }

            //outer ear
            s.set_points(freqVec_, outerEardB_);
            for(unsigned int i=0; i<freqs.size(); i++)
            {
                if(freqs[i]>=freqVec_[40])
                    response[i] += outerEardB_[40];
                else
                    response[i] += s(freqs[i]);
            }

            return 0;
        }
    }
}

