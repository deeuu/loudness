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

#ifdef __linux__
#include "Timer.h"

namespace loudness{

    Timer::Timer(string clock)
    {
        if(clock=="CPU")
            id_ = CLOCK_PROCESS_CPUTIME_ID;
        else if(clock=="WALL")
            id_ = CLOCK_MONOTONIC;
        else
            LOUDNESS_ERROR("Timer: Not a clock!");
    };
    Timer::~Timer(){};

    void Timer::tic()
    {
        clock_gettime(id_, &time1_);
    }


    //See: http://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/
    void Timer::toc()
    {
        clock_gettime(id_, &time2_);

	if ((time2_.tv_nsec-time1_.tv_nsec)<0)
        {
            elapsedTime_ = (time2_.tv_sec-time1_.tv_sec-1) +
                (1e9+time2_.tv_nsec-time1_.tv_nsec)*1e-9;
	}
        else
        {
            elapsedTime_ = (time2_.tv_sec-time1_.tv_sec) +
                (time2_.tv_nsec-time1_.tv_nsec)*1e-9;
	}

    }

    Real Timer::getElapsedTime() const
    {
        return elapsedTime_;
    }

    long int Timer::getResolution()
    {
        timespec res;
        clock_getres(id_, &res);
        return res.tv_nsec;
    }
}
#endif
