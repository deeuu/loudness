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

#ifndef  USEFULFUNCTIONS_H
#define  USEFULFUNCTIONS_H

#include <math.h>
#define PI 3.14159265358979323846264338327
#define LOW_LIMIT_POWER 1e-10
#define LOW_LIMIT_DB -100

namespace loudness{

    /** Return the larger of a or b. */
    template <typename Type>
    inline Type Max (Type const& a, Type const&b )
    {
        return a < b ? b : a;
    }

    /** Return the smaller of a or b. */
    template <typename Type>
    inline Type Min (Type const& a, Type const&b )
    {
        return a < b ? a : b;
    }

    /** Returns true if value is positive and less than upper. */
    template <typename Type>
    inline bool isPositiveAndLessThanUpper(Type const& value, Type const& upper)
    {
        return value>=0 && value<upper;
    }
}

#endif 
