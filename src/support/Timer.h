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
#ifndef TIMER_H
#define TIMER_H

#include "Common.h"
#include <time.h>

namespace loudness{

    /**
     * @class Timer
     *
     * @brief A simple class for tic/toc style perfomance timing.
     *
     * There are two clocks available:
     * 1. "CPU" (CLOCK_PROCESS_CPUTIME_ID) - A high resolution timer for measuring
     * the CPU time consumed by the process.
     * 2. "WALL" (CLOCK_MONOTONIC) - Relative real time.
     *
     * These clocks are accessed using clock_gettime().
     *
     * See: http://linux.die.net/man/3/clock_gettime
     *
     */
    class Timer
    {
    public:
        /**
         * @brief Construct a Timer object using a specific clock.
         *
         * @param clock A string ("CPU" or "WALL") specifying the clock to use.
         */
        Timer(string clock="CPU");
        ~Timer();

        /**
         * @brief Gets the current time of the clock.
         */
        void tic();

        /**
         * @brief Computes the elapsed time since the most recent call to tic();
         */
        void toc();

        /**
         * @brief Returns the elapsed time as measured by toc() in seconds.
         */
        Real getElapsedTime() const;

        /**
         * @brief Returns the resolution of the clock.
         */
        long int getResolution();

    protected:
        clockid_t id_;
        Real elapsedTime_;
        timespec time1_, time2_;
    };
}

#endif
#endif
