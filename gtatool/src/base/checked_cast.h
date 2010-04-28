/*
 * checked_cast.h
 *
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file checked_cast.h
 *
 * This class provides a static cast between integer types that throws an
 * exception if over- or underflow would occur.
 */

#ifndef CHECKED_CAST_H
#define CHECKED_CAST_H

#include <cerrno>
#include <limits>

#include "exc.h"


/* internal function; do not call directly. */

template<typename TO, typename FROM>
TO _checked_cast(FROM x) throw (exc)
{
    // The goal of this case differentiation is to
    // a) help the compiler to optimize unnecessary checks away
    // b) avoid compiler warnings like 'comparison of signed and unsigned'
    if (std::numeric_limits<FROM>::is_signed && std::numeric_limits<TO>::is_signed)
    {
        if (sizeof(FROM) > sizeof(TO)
                && (x < std::numeric_limits<TO>::min()
                    || x > std::numeric_limits<TO>::max()))
        {
            throw exc(ERANGE);
        }
    }
    else if (!std::numeric_limits<FROM>::is_signed && !std::numeric_limits<TO>::is_signed)
    {
        if (sizeof(FROM) >= sizeof(TO) && x > static_cast<FROM>(std::numeric_limits<TO>::max()))
        {
            throw exc(ERANGE);
        }
    }
    else if (std::numeric_limits<FROM>::is_signed)      // TO is unsigned
    {
        if (x < static_cast<FROM>(std::numeric_limits<TO>::min())
                || (sizeof(FROM) > sizeof(TO) && x > static_cast<FROM>(std::numeric_limits<TO>::max())))
        {
            throw exc(ERANGE);
        }
    }
    else        // FROM is unsigned, TO is signed
    {
        if (sizeof(FROM) >= sizeof(TO) && x > static_cast<FROM>(std::numeric_limits<TO>::max()))
        {
            throw exc(ERANGE);
        }
    }
    return x;
}

/**
 * \param x     Integer value.
 * \return      Casted integer value.
 *
 * Cast the integer value \a x to the given type.
 * If over- oder underflow would occur, this function throws exc(ERANGE).
 */

template<typename T>
T checked_cast(bool x) throw (exc)
{
    return x ? 1 : 0;
}

template<typename T>
T checked_cast(signed char x) throw (exc)
{
    return _checked_cast<T, signed char>(x);
}

template<typename T>
T checked_cast(unsigned char x) throw (exc)
{
    return _checked_cast<T, unsigned char>(x);
}

template<typename T>
T checked_cast(short x) throw (exc)
{
    return _checked_cast<T, short>(x);
}

template<typename T>
T checked_cast(unsigned short x) throw (exc)
{
    return _checked_cast<T, unsigned short>(x);
}

template<typename T>
T checked_cast(int x) throw (exc)
{
    return _checked_cast<T, int>(x);
}

template<typename T>
T checked_cast(unsigned int x) throw (exc)
{
    return _checked_cast<T, unsigned int>(x);
}

template<typename T>
T checked_cast(long x) throw (exc)
{
    return _checked_cast<T, long>(x);
}

template<typename T>
T checked_cast(unsigned long x) throw (exc)
{
    return _checked_cast<T, unsigned long>(x);
}

template<typename T>
T checked_cast(long long x) throw (exc)
{
    return _checked_cast<T, long long>(x);
}

template<typename T>
T checked_cast(unsigned long long x) throw (exc)
{
    return _checked_cast<T, unsigned long long>(x);
}

#endif
