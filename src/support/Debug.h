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

#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <iomanip>

//#define DEBUG

#define GET_MACRO(_1,_2,NAME,...) NAME
#define LOUDNESS_ASSERT(...) GET_MACRO(__VA_ARGS__, LOUDNESS_ASSERT2, LOUDNESS_ASSERT1)(__VA_ARGS__)

#if defined(DEBUG)
#define LOUDNESS_DEBUG(msg) do {std::cerr << msg << std::endl;} while (false)
//overloading macros: http://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments

#define LOUDNESS_ASSERT1(condition) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << std::endl; \
            std::exit(EXIT_FAILURE); \
        } \
    } while (false)

#define LOUDNESS_ASSERT2(condition, msg) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << msg << std::endl; \
            std::exit(EXIT_FAILURE); \
        } \
    } while (false)
#else
#define LOUDNESS_DEBUG(msg) do {} while (false)
#define LOUDNESS_ASSERT1(condition) do {} while (false)
#define LOUDNESS_ASSERT2(condition, msg) do {} while (false)
#endif
#if defined(PDEBUG)
#define LOUDNESS_PROCESS_DEBUG(msg) do {std::cerr << msg << std::endl;} while (false)
#else
#define LOUDNESS_PROCESS_DEBUG(msg) do {} while (false)
#endif
//exceptions (need proper class)
#define LOUDNESS_ERROR(msg) std::cerr << msg << std::endl;
//general concerns and corrections
#define LOUDNESS_WARNING(msg) std::cerr << msg << std::endl;

#endif
