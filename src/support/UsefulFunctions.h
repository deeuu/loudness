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

#include <cmath>
#include <vector>
#define PI 3.14159265358979323846264338327
#define LOWER_LIMIT_DB -100

namespace loudness{

    /** Factorial of a number. */
    template <typename Type>
    inline Type factorial (Type const& value)
    {
        return (value <= 1) ? 1 : value * factorial (value - 1);
    }

    /** Return the larger of a or b. */
    template <typename Type>
    inline Type max (Type const& a, Type const&b )
    {
        return a < b ? b : a;
    }

    /** Return the smaller of a or b. */
    template <typename Type>
    inline Type min (Type const& a, Type const&b )
    {
        return a < b ? a : b;
    }

    /** Return the absolute value of a. */
    template <typename Type>
    inline Type abs (Type const& a)
    {
        return a < 0 ? -a : a;
    }

    /** Returns true if value is positive and less than upper. */
    template <typename Type>
    inline bool isPositiveAndLessThanUpper(Type const& value, Type const& upper)
    {
        return value>=0 && value<upper;
    }

    template <typename Type, typename Type2>
    bool anyAscendingValues(const Type* data, Type2 length)
    {
        bool ascending = false;
        while(length-- > 1 && !ascending)
            ascending = *data < *(++data);
        return ascending;
    }

    template <typename Type, typename Type2>
    bool anyDescendingValues(const Type* data, Type2 length)
    {
        bool descending = false;
        while(length-- > 1 && !descending)
            descending = *data > *(++data);
        return descending;
    }

    template <typename Type>
    bool anyAscendingValues(const std::vector<Type>& data)
    {
        return anyAscendingValues(&data[0], data.size());
    }

    template <typename Type>
    bool anyDescendingValues(const std::vector<Type>& data)
    {
        return anyDescendingValues(&data[0], data.size());
    }

    template <typename Type>
    Type nextPowerOfTwo(const Type& value)
    {
        return std::pow (2, std::ceil (std::log2 (value) ) );
    }

    /** Rounds value to a number of decimal places (default is 0) */
    template <typename Type>
    Type round(const Type& value, int nDecimals = 0)
    {
        int c = std::pow (10, nDecimals);
        return Type (std::floor (value * c + 0.5) / c);
    }

    /** Convert decibels to amplitude.
     *  If decibels is greater than minValue (default LOWER_LIMIT_DB),
     *  amplitude is calculated,
     *  otherwise, clipValue is returned (default zero).
     */
    template <typename Type>
    Type decibelsToAmplitude (const Type decibels,
                              const Type minValue = (Type)LOWER_LIMIT_DB,
                              const Type clipValue = (Type)0.0)
    {
        return decibels > minValue ? std::pow (10, decibels / 20.0) : clipValue;
    }

    /** Convert amplitude to decibels.
     *  If amplitude is greater than minValue, the decibel is calculated,
     *  otherwise, clipValue is returned.
     */
    template <typename Type>
    Type amplitudeToDecibels(const Type amplitude,
                             const Type minValue = (Type)0.0,
                             const Type clipValue = (Type)LOWER_LIMIT_DB)
    {
        return amplitude > minValue ? 20 * std::log10 (amplitude) : clipValue;
    }
    /** Convert decibels to power.
     *  If decibels is greater than minValue, power is calculated,
     *  otherwise, clipValue is returned.
     */
    template <typename Type>
    Type decibelsToPower (const Type decibels,
                          const Type minValue = (Type)LOWER_LIMIT_DB,
                          const Type clipValue = (Type)0.0)
    {
        return decibels > minValue ? std::pow (10, decibels / 10.0) : clipValue;
    }

    /** Convert power to decibels.
     *  If power is greater than minValue, the decibel is calculated,
     *  otherwise, clipValue is returned.
     */
    template <typename Type>
    Type powerToDecibels (const Type power,
                          const Type minValue = (Type)0.0,
                          const Type clipValue = (Type)LOWER_LIMIT_DB)
    {
        return power > minValue ? 10 * std::log10 (power) : clipValue;
    }
}

#endif 
