/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014, 2017
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

#include "config.h"

#include <sstream>
#include <limits>
#include <cstdio>
#include <cstdint>
#include <cctype>
#include <cmath>

#include <gta/gta.hpp>

#include "base/dbg.h"
#include "base/msg.h"
#include "base/blb.h"
#include "base/opt.h"
#include "base/fio.h"
#include "base/str.h"
#include "base/chk.h"

/* Get maximum-width types before including lib.h */
#if !defined(HAVE_INT128_T) && defined(HAVE___INT128)
typedef __int128 max_int_t;
#else
typedef intmax_t max_int_t;
#endif
#if !defined(HAVE_UINT128_T) && defined(HAVE_UNSIGNED___INT128)
typedef unsigned __int128 max_uint_t;
#else
typedef uintmax_t max_uint_t;
#endif
#ifdef HAVE___FLOAT128
typedef __float128 max_float_t;
#else
typedef long double max_float_t;
#endif

#include "lib.h"


static max_uint_t to_max_uint(const void *val, gta::type type, max_uint_t normalization_max)
{
    max_uint_t x;
    switch (type)
    {
    case gta::int8:
        {
            int8_t v;
            std::memcpy(&v, val, sizeof(int8_t));
            x = (v < 0 ? 0 : v);
        }
        break;
    case gta::uint8:
        {
            uint8_t v;
            std::memcpy(&v, val, sizeof(uint8_t));
            x = v;
        }
        break;
    case gta::int16:
        {
            int16_t v;
            std::memcpy(&v, val, sizeof(int16_t));
            x = (v < 0 ? 0 : v);
        }
        break;
    case gta::uint16:
        {
            uint16_t v;
            std::memcpy(&v, val, sizeof(uint16_t));
            x = v;
        }
        break;
    case gta::int32:
        {
            int32_t v;
            std::memcpy(&v, val, sizeof(int32_t));
            x = (v < 0 ? 0 : v);
        }
        break;
    case gta::uint32:
        {
            uint32_t v;
            std::memcpy(&v, val, sizeof(uint32_t));
            x = v;
        }
        break;
    case gta::int64:
        {
            int64_t v;
            std::memcpy(&v, val, sizeof(int64_t));
            x = (v < 0 ? 0 : v);
        }
        break;
    case gta::uint64:
        {
            uint64_t v;
            std::memcpy(&v, val, sizeof(uint64_t));
            x = v;
        }
        break;
#ifdef HAVE_INT128_T
    case gta::int128:
        {
            int128_t v;
            std::memcpy(&v, val, sizeof(int128_t));
            x = (v < 0 ? 0 : v);
        }
        break;
#endif
#ifdef HAVE_UINT128_T
    case gta::uint128:
        {
            uint128_t v;
            std::memcpy(&v, val, sizeof(uint128_t));
            x = v;
        }
        break;
#endif
    case gta::float32:
    case gta::cfloat32:
        {
            float v;
            std::memcpy(&v, val, sizeof(float));
            if (v < 0.0f)
                v = 0.0f;
            else if (v > 1.0f)
                v = 1.0f;
            if (normalization_max)
                v *= normalization_max;
            x = ((!std::isfinite(v) || v < 0) ? 0 : v);
        }
        break;
    case gta::float64:
    case gta::cfloat64:
        {
            double v;
            std::memcpy(&v, val, sizeof(double));
            if (v < 0.0)
                v = 0.0;
            else if (v > 1.0)
                v = 1.0;
            if (normalization_max)
                v *= normalization_max;
            x = ((!std::isfinite(v) || v < 0) ? 0 : v);
        }
        break;
#ifdef HAVE_FLOAT128_T
    case gta::float128:
    case gta::cfloat128:
        {
            float128_t v;
            std::memcpy(&v, val, sizeof(float128_t));
            if (v < 0.0)
                v = 0.0;
            else if (v > 1.0)
                v = 1.0;
            if (normalization_max)
                v *= normalization_max;
#ifdef LONG_DOUBLE_IS_IEEE_754_QUAD
            x = ((!std::isfinite(v) || v < 0) ? 0 : v);
#else
            x = ((!isinfq(v) || v < 0) ? 0 : v);
#endif
        }
        break;
#endif
    default:
        // cannot happen
        assert(false);
        x = 0;
        break;
    }
    return x;
}

static max_int_t to_max_int(const void *val, gta::type type, max_int_t normalization_min, max_int_t normalization_max)
{
    max_int_t x;
    switch (type)
    {
    case gta::int8:
        {
            int8_t v;
            std::memcpy(&v, val, sizeof(int8_t));
            x = v;
        }
        break;
    case gta::uint8:
        {
            uint8_t v;
            std::memcpy(&v, val, sizeof(uint8_t));
            x = v;
        }
        break;
    case gta::int16:
        {
            int16_t v;
            std::memcpy(&v, val, sizeof(int16_t));
            x = v;
        }
        break;
    case gta::uint16:
        {
            uint16_t v;
            std::memcpy(&v, val, sizeof(uint16_t));
            x = v;
        }
        break;
    case gta::int32:
        {
            int32_t v;
            std::memcpy(&v, val, sizeof(int32_t));
            x = v;
        }
        break;
    case gta::uint32:
        {
            uint32_t v;
            std::memcpy(&v, val, sizeof(uint32_t));
            x = v;
        }
        break;
    case gta::int64:
        {
            int64_t v;
            std::memcpy(&v, val, sizeof(int64_t));
            x = v;
        }
        break;
    case gta::uint64:
        {
            uint64_t v;
            std::memcpy(&v, val, sizeof(uint64_t));
            x = v;
        }
        break;
#ifdef HAVE_INT128_T
    case gta::int128:
        {
            int128_t v;
            std::memcpy(&v, val, sizeof(int128_t));
            x = v;
        }
        break;
#endif
#ifdef HAVE_UINT128_T
    case gta::uint128:
        {
            uint128_t v;
            std::memcpy(&v, val, sizeof(uint128_t));
            x = v;
        }
        break;
#endif
    case gta::float32:
    case gta::cfloat32:
        {
            float v;
            std::memcpy(&v, val, sizeof(float));
            if (v < -1.0f)
                v = -1.0f;
            else if (v > 1.0f)
                v = 1.0f;
            if (normalization_min && v < 0)
                v *= -1.0f * normalization_min;
            if (normalization_max && v > 0)
                v *= normalization_max;
            x = (!std::isfinite(v) ? 0 : v);
        }
        break;
    case gta::float64:
    case gta::cfloat64:
        {
            double v;
            std::memcpy(&v, val, sizeof(double));
            if (v < -1.0)
                v = -1.0;
            else if (v > 1.0)
                v = 1.0;
            if (normalization_min && v < 0)
                v *= -1.0 * normalization_min;
            if (normalization_max && v > 0)
                v *= normalization_max;
            x = (!std::isfinite(v) ? 0 : v);
        }
        break;
#ifdef HAVE_FLOAT128_T
    case gta::float128:
    case gta::cfloat128:
        {
            float128_t v;
            std::memcpy(&v, val, sizeof(float128_t));
            if (v < -1.0)
                v = -1.0;
            else if (v > 1.0)
                v = 1.0;
            if (normalization_min && v < 0)
                v *= -1.0 * normalization_min;
            if (normalization_max && v > 0)
                v *= normalization_max;
#ifdef LONG_DOUBLE_IS_IEEE_754_QUAD
            x = (!std::isfinite(v) ? 0 : v);
#else
            x = (!isinfq(v) ? 0 : v);
#endif
        }
        break;
#endif
    default:
        // cannot happen
        assert(false);
        x = 0;
        break;
    }
    return x;
}

static max_float_t to_max_float(const void *val, gta::type type, bool normalize)
{
    max_float_t x;
    switch (type)
    {
    case gta::int8:
        {
            int8_t v;
            std::memcpy(&v, val, sizeof(int8_t));
            x = v;
            if (normalize && v < 0)
                x /= -1.0 * std::numeric_limits<int8_t>::min();
            else if (normalize && v > 0)
                x /= std::numeric_limits<int8_t>::max();
        }
        break;
    case gta::uint8:
        {
            uint8_t v;
            std::memcpy(&v, val, sizeof(uint8_t));
            x = v;
            if (normalize && v > 0)
                x /= std::numeric_limits<uint8_t>::max();
        }
        break;
    case gta::int16:
        {
            int16_t v;
            std::memcpy(&v, val, sizeof(int16_t));
            x = v;
            if (normalize && v < 0)
                x /= -1.0 * std::numeric_limits<int16_t>::min();
            else if (normalize && v > 0)
                x /= std::numeric_limits<int16_t>::max();
        }
        break;
    case gta::uint16:
        {
            uint16_t v;
            std::memcpy(&v, val, sizeof(uint16_t));
            x = v;
            if (normalize && v > 0)
                x /= std::numeric_limits<uint16_t>::max();
        }
        break;
    case gta::int32:
        {
            int32_t v;
            std::memcpy(&v, val, sizeof(int32_t));
            x = v;
            if (normalize && v < 0)
                x /= -1.0 * std::numeric_limits<int32_t>::min();
            else if (normalize && v > 0)
                x /= std::numeric_limits<int32_t>::max();
        }
        break;
    case gta::uint32:
        {
            uint32_t v;
            std::memcpy(&v, val, sizeof(uint32_t));
            x = v;
            if (normalize && v > 0)
                x /= std::numeric_limits<uint32_t>::max();
        }
        break;
    case gta::int64:
        {
            int64_t v;
            std::memcpy(&v, val, sizeof(int64_t));
            x = v;
            if (normalize && v < 0)
                x /= -1.0 * std::numeric_limits<int64_t>::min();
            else if (normalize && v > 0)
                x /= std::numeric_limits<int64_t>::max();
        }
        break;
    case gta::uint64:
        {
            uint64_t v;
            std::memcpy(&v, val, sizeof(uint64_t));
            x = v;
            if (normalize && v > 0)
                x /= std::numeric_limits<uint64_t>::max();
        }
        break;
#ifdef HAVE_INT128_T
    case gta::int128:
        {
            int128_t v;
            std::memcpy(&v, val, sizeof(int128_t));
            x = v;
            if (normalize && v < 0)
                x /= -1.0 * INT128_MIN;
            else if (normalize && v > 0)
                x /= INT128_MAX;
        }
        break;
#endif
#ifdef HAVE_UINT128_T
    case gta::uint128:
        {
            uint128_t v;
            std::memcpy(&v, val, sizeof(uint128_t));
            x = v;
            if (normalize && v > 0)
                x /= UINT128_MAX;
        }
        break;
#endif
    case gta::float32:
    case gta::cfloat32:
        {
            float v;
            std::memcpy(&v, val, sizeof(float));
            x = v;
        }
        break;
    case gta::float64:
    case gta::cfloat64:
        {
            double v;
            std::memcpy(&v, val, sizeof(double));
            x = v;
        }
        break;
#ifdef HAVE_FLOAT128_T
    case gta::float128:
    case gta::cfloat128:
        {
            float128_t v;
            std::memcpy(&v, val, sizeof(float128_t));
            x = v;
        }
        break;
#endif
    default:
        // cannot happen
        assert(false);
        x = 0;
        break;
    }
    return x;
}

static void to_max_cfloat(max_float_t c[2], const void *val, gta::type type, bool normalize)
{
    switch (type)
    {
    case gta::int8:
        {
            int8_t v;
            std::memcpy(&v, val, sizeof(int8_t));
            c[0] = v;
            c[1] = 0;
            if (normalize && v < 0)
                c[0] /= -1.0 * std::numeric_limits<int8_t>::min();
            else if (normalize && v > 0)
                c[0] /= std::numeric_limits<int8_t>::max();
        }
        break;
    case gta::uint8:
        {
            uint8_t v;
            std::memcpy(&v, val, sizeof(uint8_t));
            c[0] = v;
            c[1] = 0;
            if (normalize && v > 0)
                c[0] /= std::numeric_limits<uint8_t>::max();
        }
        break;
    case gta::int16:
        {
            int16_t v;
            std::memcpy(&v, val, sizeof(int16_t));
            c[0] = v;
            c[1] = 0;
            if (normalize && v < 0)
                c[0] /= -1.0 * std::numeric_limits<int16_t>::min();
            else if (normalize && v > 0)
                c[0] /= std::numeric_limits<int16_t>::max();
        }
        break;
    case gta::uint16:
        {
            uint16_t v;
            std::memcpy(&v, val, sizeof(uint16_t));
            c[0] = v;
            c[1] = 0;
            if (normalize && v > 0)
                c[0] /= std::numeric_limits<uint16_t>::max();
        }
        break;
    case gta::int32:
        {
            int32_t v;
            std::memcpy(&v, val, sizeof(int32_t));
            c[0] = v;
            c[1] = 0;
            if (normalize && v < 0)
                c[0] /= -1.0 * std::numeric_limits<int32_t>::min();
            else if (normalize && v > 0)
                c[0] /= std::numeric_limits<int32_t>::max();
        }
        break;
    case gta::uint32:
        {
            uint32_t v;
            std::memcpy(&v, val, sizeof(uint32_t));
            c[0] = v;
            c[1] = 0;
            if (normalize && v > 0)
                c[0] /= std::numeric_limits<uint32_t>::max();
        }
        break;
    case gta::int64:
        {
            int64_t v;
            std::memcpy(&v, val, sizeof(int64_t));
            c[0] = v;
            c[1] = 0;
            if (normalize && v < 0)
                c[0] /= -1.0 * std::numeric_limits<int64_t>::min();
            else if (normalize && v > 0)
                c[0] /= std::numeric_limits<int64_t>::max();
        }
        break;
    case gta::uint64:
        {
            uint64_t v;
            std::memcpy(&v, val, sizeof(uint64_t));
            c[0] = v;
            c[1] = 0;
            if (normalize && v > 0)
                c[0] /= std::numeric_limits<uint64_t>::max();
        }
        break;
#ifdef HAVE_INT128_T
    case gta::int128:
        {
            int128_t v;
            std::memcpy(&v, val, sizeof(int128_t));
            c[0] = v;
            c[1] = 0;
            if (normalize && v < 0)
                c[0] /= -1.0 * INT128_MIN;
            else if (normalize && v > 0)
                c[0] /= INT128_MAX;
        }
        break;
#endif
#ifdef HAVE_UINT128_T
    case gta::uint128:
        {
            uint128_t v;
            std::memcpy(&v, val, sizeof(uint128_t));
            c[0] = v;
            c[1] = 0;
            if (normalize && v > 0)
                c[0] /= UINT128_MAX;
        }
        break;
#endif
    case gta::float32:
        {
            float v;
            std::memcpy(&v, val, sizeof(float));
            c[0] = v;
            c[1] = 0;
        }
        break;
    case gta::float64:
        {
            double v;
            std::memcpy(&v, val, sizeof(double));
            c[0] = v;
            c[1] = 0;
        }
        break;
#ifdef HAVE_FLOAT128_T
    case gta::float128:
        {
            float128_t v;
            std::memcpy(&v, val, sizeof(float128_t));
            c[0] = v;
            c[1] = 0;
        }
        break;
#endif
    case gta::cfloat32:
        {
            float v[2];
            std::memcpy(v, val, 2 * sizeof(float));
            c[0] = v[0];
            c[1] = v[1];
        }
        break;
    case gta::cfloat64:
        {
            double v[2];
            std::memcpy(v, val, 2 * sizeof(double));
            c[0] = v[0];
            c[1] = v[1];
        }
        break;
#ifdef HAVE_FLOAT128_T
    case gta::cfloat128:
        {
            float128_t v[2];
            std::memcpy(v, val, 2 * sizeof(float128_t));
            c[0] = v[0];
            c[1] = v[1];
        }
        break;
#endif
    default:
        // cannot happen
        assert(false);
        c[0] = 0;
        c[1] = 0;
        break;
    }
}

static void convert(void *dst, gta::type dst_type, const void *src, gta::type src_type, bool normalize)
{
    if (dst_type == gta::cfloat32 || dst_type == gta::cfloat64 || dst_type == gta::cfloat128)
    {
        max_float_t val[2];
        to_max_cfloat(val, src, src_type, normalize);
        if (dst_type == gta::cfloat32)
        {
            float v[2] = { static_cast<float>(val[0]), static_cast<float>(val[1]) };
            std::memcpy(dst, v, 2 * sizeof(float));
        }
        else if (dst_type == gta::cfloat64)
        {
            double v[2] = { static_cast<double>(val[0]), static_cast<double>(val[1]) };
            std::memcpy(dst, v, 2 * sizeof(double));
        }
#ifdef HAVE_FLOAT128_T
        else
        {
            float128_t v[2] = { static_cast<float128_t>(val[0]), static_cast<float128_t>(val[1]) };
            std::memcpy(dst, v, 2 * sizeof(float128_t));
        }
#endif
    }
    else if (dst_type == gta::float32 || dst_type == gta::float64 || dst_type == gta::float128)
    {
        max_float_t val = to_max_float(src, src_type, normalize);
        if (dst_type == gta::float32)
        {
            float v = val;
            std::memcpy(dst, &v, sizeof(float));
        }
        else if (dst_type == gta::float64)
        {
            double v = val;
            std::memcpy(dst, &v, sizeof(double));
        }
#ifdef HAVE_FLOAT128_T
        else
        {
            float128_t v = val;
            std::memcpy(dst, &v, sizeof(float128_t));
        }
#endif
    }
    else if (dst_type == gta::int8
            || dst_type == gta::int16
            || dst_type == gta::int32
            || dst_type == gta::int64
            || dst_type == gta::int128)
    {
        max_int_t normalization_min = (!normalize ? 0
                : dst_type == gta::int8  ? std::numeric_limits<int8_t>::min()
                : dst_type == gta::int16 ? std::numeric_limits<int16_t>::min()
                : dst_type == gta::int32 ? std::numeric_limits<int32_t>::min()
                : dst_type == gta::int64 ? std::numeric_limits<int64_t>::min()
                :
#ifdef HAVE_INT128_T
                INT128_MIN
#else
                0
#endif
                );
        max_int_t normalization_max = (!normalize ? 0
                : dst_type == gta::int8  ? std::numeric_limits<int8_t>::max()
                : dst_type == gta::int16 ? std::numeric_limits<int16_t>::max()
                : dst_type == gta::int32 ? std::numeric_limits<int32_t>::max()
                : dst_type == gta::int64 ? std::numeric_limits<int64_t>::max()
                :
#ifdef HAVE_INT128_T
                INT128_MAX
#else
                0
#endif
                );
        max_int_t val = to_max_int(src, src_type, normalization_min, normalization_max);
        if (dst_type == gta::int8)
        {
            int8_t v = val;
            std::memcpy(dst, &v, sizeof(int8_t));
        }
        else if (dst_type == gta::int16)
        {
            int16_t v = val;
            std::memcpy(dst, &v, sizeof(int16_t));
        }
        else if (dst_type == gta::int32)
        {
            int32_t v = val;
            std::memcpy(dst, &v, sizeof(int32_t));
        }
        else if (dst_type == gta::int64)
        {
            int64_t v = val;
            std::memcpy(dst, &v, sizeof(int64_t));
        }
#ifdef HAVE_INT128_T
        else
        {
            int128_t v = val;
            std::memcpy(dst, &v, sizeof(int128_t));
        }
#endif
    }
    else // gta::uint{8,16,32,64}
    {
        max_uint_t normalization_max = (!normalize ? 0
                : dst_type == gta::uint8  ? std::numeric_limits<uint8_t>::max()
                : dst_type == gta::uint16 ? std::numeric_limits<uint16_t>::max()
                : dst_type == gta::uint32 ? std::numeric_limits<uint32_t>::max()
                : dst_type == gta::uint64 ? std::numeric_limits<uint64_t>::max()
                :
#ifdef HAVE_UINT128_T
                UINT128_MAX
#else
                0
#endif
                );
        max_uint_t val = to_max_uint(src, src_type, normalization_max);
        if (dst_type == gta::uint8)
        {
            uint8_t v = val;
            std::memcpy(dst, &v, sizeof(uint8_t));
        }
        else if (dst_type == gta::uint16)
        {
            uint16_t v = val;
            std::memcpy(dst, &v, sizeof(uint16_t));
        }
        else if (dst_type == gta::uint32)
        {
            uint32_t v = val;
            std::memcpy(dst, &v, sizeof(uint32_t));
        }
        else if (dst_type == gta::uint64)
        {
            uint64_t v = val;
            std::memcpy(dst, &v, sizeof(uint64_t));
        }
#ifdef HAVE_UINT128_T
        else
        {
            uint128_t v = val;
            std::memcpy(dst, &v, sizeof(uint128_t));
        }
#endif
    }
}

extern "C" void gtatool_component_convert_help(void)
{
    msg::req_txt(
            "component-convert -c|--components=<c0>[,<c1>[,...]] [-n|--normalize] [<files>...]\n"
            "\n"
            "Converts the array element components of the given GTAs to the given types, "
            "and writes the resulting GTA to standard output.\n"
            "If --normalize is given, the range of an integer type is normalized when converting it to a "
            "floating point type (to [0,1] for unsigned integers, and to [-1,1] for signed integers), and this "
            "conversion is reverted when converting a floating point type to an integer type. "
            "Note that normalization may lose information not only due to type limitations, but also due to clamping.\n"
            "Example: component-convert -c uint8,uint8,uint8 hdr.gta > rgb.gta");
}

extern "C" int gtatool_component_convert(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::string components("components", 'c', opt::required);
    options.push_back(&components);
    opt::flag normalize("normalize", 'n', opt::optional);
    options.push_back(&normalize);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_component_convert_help();
        return 0;
    }

    try
    {
        std::vector<gta::type> comp_types;
        std::vector<uintmax_t> comp_sizes;
        typelist_from_string(components.value(), &comp_types, &comp_sizes);
        for (size_t i = 0; i < comp_types.size(); i++)
        {
            if (comp_types[i] == gta::blob)
            {
                throw exc("conversion to type "
                        + type_to_string(comp_types[i], comp_sizes[0])
                        + " is currently not supported");
            }
            if (false
#ifndef HAVE_INT128_T
                    || comp_types[i] == gta::int128
#endif
#ifndef HAVE_UINT128_T
                    || comp_types[i] == gta::uint128
#endif
#ifndef HAVE_FLOAT128_T
                    || comp_types[i] == gta::float128
                    || comp_types[i] == gta::cfloat128
#endif
                    )
            {
                throw exc("conversion to type "
                        + type_to_string(comp_types[i], 0)
                        + " is not supported on this platform");
            }
        }

        array_loop_t array_loop;
        gta::header hdri, hdro;
        std::string namei, nameo;
        array_loop.start(arguments, "");
        while (array_loop.read(hdri, namei))
        {
            if (hdri.components() != comp_types.size())
            {
                throw exc(namei + ": number of components does not match");
            }
            for (uintmax_t i = 0; i < hdri.components(); i++)
            {
                if (hdri.component_type(i) == gta::blob)
                {
                    throw exc(namei + ": conversion from type "
                            + type_to_string(hdri.component_type(i), hdri.component_size(i))
                            + " is currently not supported");
                }
                if (false
#ifndef HAVE_INT128_T
                        || hdri.component_type(i) == gta::int128
#endif
#ifndef HAVE_UINT128_T
                        || hdri.component_type(i) == gta::uint128
#endif
#ifndef HAVE_FLOAT128_T
                        || hdri.component_type(i) == gta::float128
                        || hdri.component_type(i) == gta::cfloat128
#endif
                   )
                {
                    throw exc(namei + ": conversion from type "
                            + type_to_string(hdri.component_type(i), hdri.component_size(i))
                            + " is not supported on this platform");
                }
            }

            // Convert components
            hdro = hdri;
            hdro.set_compression(gta::none);
            hdro.set_components(comp_types.size(), &(comp_types[0]), comp_sizes.size() == 0 ? NULL : &(comp_sizes[0]));
            for (uintmax_t i = 0; i < hdro.components(); i++)
            {
                hdro.component_taglist(i) = hdri.component_taglist(i);
            }
            array_loop.write(hdro, nameo);
            element_loop_t element_loop;
            array_loop.start_element_loop(element_loop, hdri, hdro);
            blob element_out(checked_cast<size_t>(hdro.element_size()));
            for (uintmax_t e = 0; e < hdro.elements(); e++)
            {
                const void *src = element_loop.read();
                for (uintmax_t i = 0; i < hdro.components(); i++)
                {
                    convert(hdro.component(element_out.ptr(), i),
                            hdro.component_type(i),
                            hdri.component(src, i),
                            hdri.component_type(i),
                            normalize.value());
                }
                element_loop.write(element_out.ptr());
            }
        }
        array_loop.finish();
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
