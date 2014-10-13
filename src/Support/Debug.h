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

//I need to throw an exception
#define LOUDNESS_ERROR(msg) std::cerr << msg << std::endl;
//general concerns
#define LOUDNESS_WARNING(msg) std::cerr << msg << std::endl;

// If none of the NO_DEBUG are defined we enable debugging
#if defined(DEBUG)

#define LOUDNESS_DEBUG(msg) std::cerr << msg << std::endl;

// else we do nothing
#else

#define LOUDNESS_DEBUG(msg)

#endif

#endif
