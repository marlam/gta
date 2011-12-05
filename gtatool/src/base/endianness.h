/*
 * Copyright (C) 2009, 2010, 2011
 * Martin Lambers <marlam@marlam.de>
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
 * \file endianness.h
 * \brief Swap endianness.
 *
 * Swap endianness.
 */


#ifndef ENDIANNESS_H
#define ENDIANNESS_H

#include <cstring>
#include <stdint.h>


namespace endianness
{
    typedef enum
    {
        little = 0,
        big = 1
    } type;

    const type endianness =
#if WORDS_BIGENDIAN
        big
#else
        little
#endif
        ;

    /**
     * \param ptr       Pointer to a 16 bit value.
     *
     * Swaps the endianness of a 16 bit value.
     */
    inline void swap16(void *ptr)
    {
        uint16_t x;
        memcpy(&x, ptr, sizeof(uint16_t));
        x = (x << uint16_t(8)) | (x >> uint16_t(8));
        memcpy(ptr, &x, sizeof(uint16_t));
    }

    /**
     * \param ptr       Pointer to a 32 bit value.
     *
     * Swaps the endianness of a 32 bit value.
     */
    inline void swap32(void *ptr)
    {
        uint32_t x;
        memcpy(&x, ptr, sizeof(uint32_t));
        x =   ((x                        ) << uint32_t(24))
            | ((x & uint32_t(0x0000ff00U)) << uint32_t(8))
            | ((x & uint32_t(0x00ff0000U)) >> uint32_t(8))
            | ((x                        ) >> uint32_t(24));
        memcpy(ptr, &x, sizeof(uint32_t));
    }

    /**
     * \param ptr       Pointer to a 64 bit value.
     *
     * Swaps the endianness of a 64 bit value.
     */
    inline void swap64(void *ptr)
    {
        uint64_t x;
        memcpy(&x, ptr, sizeof(uint64_t));
        x =   ((x                                  ) << uint64_t(56))
            | ((x & uint64_t(0x000000000000ff00ULL)) << uint64_t(40))
            | ((x & uint64_t(0x0000000000ff0000ULL)) << uint64_t(24))
            | ((x & uint64_t(0x00000000ff000000ULL)) << uint64_t(8))
            | ((x & uint64_t(0x000000ff00000000ULL)) >> uint64_t(8))
            | ((x & uint64_t(0x0000ff0000000000ULL)) >> uint64_t(24))
            | ((x & uint64_t(0x00ff000000000000ULL)) >> uint64_t(40))
            | ((x                                  ) >> uint64_t(56));
        memcpy(ptr, &x, sizeof(uint64_t));
    }

    /**
     * \param ptr       Pointer to a 128 bit value.
     *
     * Swaps the endianness of a 128 bit value.
     */
    inline void swap128(void *ptr)
    {
        /* We cannot expect uint128_t to be available. */
        uint64_t x[2];
        memcpy(x, ptr, 2 * sizeof(uint64_t));
        swap64(x + 0);
        swap64(x + 1);
        uint64_t tmp = x[0];
        x[0] = x[1];
        x[1] = tmp;
        memcpy(ptr, x, 2 * sizeof(uint64_t));
    }
}

#endif
