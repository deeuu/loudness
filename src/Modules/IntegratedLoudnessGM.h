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

#ifndef INTEGRATEDLOUDNESSGM_H
#define INTEGRATEDLOUDNESSGM_H

#include "../Support/Module.h"

namespace loudness{

    /**
     * @class IntegratedLoudnessGM
     *
     * @brief Given a specific loudness pattern this class computes the
     * following loudness values: Instantaneous loudness (integrated specific
     * loudness) Short-term loudness (smoothed instantaneous loudness) Long-term
     * loudness (smoothed short-term loudness).
     *
     * This implementation follows: Glasberg, B. R., & Moore, B. C.  J. (2002).
     * A Model of Loudness Applicable to Time-Varying Sounds.  Journal of the
     * Audio Engineering Society, 50(5), 331–342.
     *
     * @todo  Allow for modified time constants as specified in M, G & S 2003 paper.
     *
     * @author Dominic Ward
     */
    class IntegratedLoudnessGM : public Module
    {
    public:
        enum SMOOTHTIMES{
            GM02 = 0,
            GM03 = 1,
            CH12 = 2
        };
 
        IntegratedLoudnessGM(const string& author, Real cParam);

        virtual ~IntegratedLoudnessGM();

        void configureSmoothingTimes(const string& author);
        void setAttackTimeSTL(Real attackTimeSTL);
        void setReleaseTimeSTL(Real releaseTimeSTL);
        void setAttackTimeLTL(Real attackTimeLTL);
        void setReleaseTimeLTL(Real releaseTimeLTL);

    private:
        virtual bool initializeInternal(const SignalBank &input);
        virtual void processInternal(const SignalBank &input);
        virtual void resetInternal();

        Real cParam_;
        Real attackTimeSTL_, releaseTimeSTL_, attackTimeLTL_, releaseTimeLTL_;
        Real attackSTLCoef_, releaseSTLCoef_, attackLTLCoef_, releaseLTLCoef_;
        Real camStep_, timeStep_;
    };
}
#endif
