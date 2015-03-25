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

#ifndef  COMMON_H
#define  COMMON_H

#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include "Debug.h"
#include "UsefulFunctions.h"

/*
 * Expose objects
 */
namespace loudness{
    using std::vector;
    using std::string;
    using std::unique_ptr;
}

/*
 * Types
 */
typedef double Real;
typedef unsigned int uint;
typedef std::vector<Real> RealVec;
typedef std::vector<std::vector<Real> > RealVecVec;
typedef std::vector<int> IntVec;
typedef std::vector<Real>::iterator RealIter;

#endif  
