/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2013, 2014
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

#include <cmath>

#include <gta/gta.hpp>

#include "base/msg.h"
#include "base/opt.h"
#include "base/fio.h"
#include "base/chk.h"

#include "lib.h"


extern "C" void gtatool_diff_help(void)
{
    msg::req_txt(
            "diff [-a|--absolute] [-f|--force] <file0> <file1>\n"
            "\n"
            "Compute the differences between two GTA streams.\n"
            "The GTAs must be compatible in dimensions and component types. This command produces "
            "output GTAs of the same kind. Each component will contain the difference between "
            "the corresponding components in the input GTAs (<file0> - <file1>).\n"
            "If -a is given, the absolute difference is computed.\n"
            "Run the output GTAs through the info command with the -s option to gather statistics.\n"
            "Beware of limitations of the integer type range! If a difference cannot be represented "
            "in the given component type (e.g. 10 - 20 in uint8), this command will abort by default. "
            "Use -f to force clamping of values to the representable range instead, or use the "
            "component-convert command to work with different component types.\n"
            "Example: diff a.gta b.gta > diff.gta");
}

template<typename T>
static void signed_int_diff(bool absolute, bool force, const void* c0, const void* c1, void* d)
{
    T x, y, z;
    std::memcpy(&x, c0, sizeof(T));
    std::memcpy(&y, c1, sizeof(T));
    try {
        z = checked_sub(x, y);
        if (absolute)
            z = checked_abs(z);
    }
    catch (std::overflow_error&) {
        if (force)
            z = std::numeric_limits<T>::max();
        else
            throw;
    }
    catch (std::underflow_error&) {
        if (force)
            z = std::numeric_limits<T>::min();
        else
            throw;
    }
    std::memcpy(d, &z, sizeof(T));
}

template<typename T>
static void unsigned_int_diff(bool absolute, bool force, const void* c0, const void* c1, void* d)
{
    T x, y, z;
    std::memcpy(&x, c0, sizeof(T));
    std::memcpy(&y, c1, sizeof(T));
    if (absolute) {
        z = (x > y ? x - y : y - x);
    } else {
        try {
            z = checked_sub(x, y);
        }
        catch (std::overflow_error&) {
            if (force)
                z = std::numeric_limits<T>::max();
            else
                throw;
        }
        catch (std::underflow_error&) {
            if (force)
                z = std::numeric_limits<T>::min();
            else
                throw;
        }
    }
    std::memcpy(d, &z, sizeof(T));
}

template<typename T>
static void float_diff(bool absolute, const void* c0, const void* c1, void* d)
{
    T x, y, z;
    std::memcpy(&x, c0, sizeof(T));
    std::memcpy(&y, c1, sizeof(T));
    z = x - y;
    if (absolute)
        z = std::abs(z);
    std::memcpy(d, &z, sizeof(T));
}

#if !defined(LONG_DOUBLE_IS_IEEE_754_QUAD) && defined(HAVE___FLOAT128)
template<>
void float_diff<float128_t>(bool absolute, const void* c0, const void* c1, void* d)
{
    float128_t x, y, z;
    std::memcpy(&x, c0, sizeof(float128_t));
    std::memcpy(&y, c1, sizeof(float128_t));
    z = x - y;
    if (absolute)
        z = fabsq(z);
    std::memcpy(d, &z, sizeof(float128_t));
}
#endif

static void diff(gta::type t, bool absolute, bool force, const void* c0, const void* c1, void* d)
{
    if (t == gta::int8)
        signed_int_diff<int8_t>(absolute, force, c0, c1, d);
    else if (t == gta::uint8)
        unsigned_int_diff<uint8_t>(absolute, force, c0, c1, d);
    else if (t == gta::int16)
        signed_int_diff<int16_t>(absolute, force, c0, c1, d);
    else if (t == gta::uint16)
        unsigned_int_diff<uint16_t>(absolute, force, c0, c1, d);
    else if (t == gta::int32)
        signed_int_diff<int32_t>(absolute, force, c0, c1, d);
    else if (t == gta::uint32)
        unsigned_int_diff<uint32_t>(absolute, force, c0, c1, d);
    else if (t == gta::int64)
        signed_int_diff<int64_t>(absolute, force, c0, c1, d);
    else if (t == gta::uint64)
        unsigned_int_diff<uint64_t>(absolute, force, c0, c1, d);
#ifdef HAVE_INT128_T
    else if (t == gta::int128)
        signed_int_diff<int128_t>(absolute, force, c0, c1, d);
#endif
#ifdef HAVE_UINT128_T
    else if (t == gta::uint128)
        unsigned_int_diff<uint128_t>(absolute, force, c0, c1, d);
#endif
    else if (t == gta::float32)
        float_diff<float>(absolute, c0, c1, d);
    else if (t == gta::float64)
        float_diff<double>(absolute, c0, c1, d);
#ifdef HAVE_FLOAT128_T
    else if (t == gta::float128)
        float_diff<float128_t>(absolute, c0, c1, d);
#endif
}

extern "C" int gtatool_diff(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::flag absolute("absolute", 'a', opt::optional);
    options.push_back(&absolute);
    opt::flag force("force", 'f', opt::optional);
    options.push_back(&force);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 2, 2, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_diff_help();
        return 0;
    }

    try
    {
        array_loop_t array_loops[2];
        gta::header hdri[2];
        std::string namei[2];

        array_loops[0].start(arguments[0], "");
        array_loops[1].start(arguments[1], "");
        while (array_loops[0].read(hdri[0], namei[0]))
        {
            if (!array_loops[1].read(hdri[1], namei[1]))
            {
                msg::wrn_txt("ignoring additional array(s) from %s", arguments[0].c_str());
                break;
            }

            if (hdri[1].components() != hdri[0].components())
            {
                throw exc(namei[1] + ": incompatible array");
            }
            for (uintmax_t c = 0; c < hdri[0].components(); c++)
            {
                if (hdri[1].component_type(c) != hdri[0].component_type(c)
                        || hdri[1].component_size(c) != hdri[0].component_size(c))
                {
                    throw exc(namei[1] + ": incompatible array");
                }
                if (hdri[1].component_type(c) != gta::int8
                        && hdri[1].component_type(c) != gta::uint8
                        && hdri[1].component_type(c) != gta::int16
                        && hdri[1].component_type(c) != gta::uint16
                        && hdri[1].component_type(c) != gta::int32
                        && hdri[1].component_type(c) != gta::uint32
                        && hdri[1].component_type(c) != gta::int64
                        && hdri[1].component_type(c) != gta::uint64
#ifdef HAVE_INT128_T
                        && hdri[1].component_type(c) != gta::int128
#endif
#ifdef HAVE_UINT128_T
                        && hdri[1].component_type(c) != gta::uint128
#endif
                        && hdri[1].component_type(c) != gta::float32
                        && hdri[1].component_type(c) != gta::float64
#ifdef HAVE_FLOAT128_T
                        && hdri[1].component_type(c) != gta::float128
#endif
                   )
                {
                    throw exc(namei[1] + ": cannot compute differences of type "
                            + type_to_string(hdri[1].component_type(c), hdri[1].component_size(c)));
                }
            }
            if (hdri[1].dimensions() != hdri[0].dimensions())
            {
                throw exc(namei[1] + ": incompatible array");
            }
            for (uintmax_t d = 0; d < hdri[0].dimensions(); d++)
            {
                if (hdri[1].dimension_size(d) != hdri[0].dimension_size(d))
                {
                    throw exc(namei[1] + ": incompatible array");
                }
            }
            
            gta::header hdro = hdri[0];
            hdro.set_compression(gta::none);
            std::string nameo;
            array_loops[0].write(hdro, nameo);
            if (hdri[1].data_size() == 0)
            {
                continue;
            }

            element_loop_t element_loops[2];
            array_loops[0].start_element_loop(element_loops[0], hdri[0], hdro);
            array_loops[1].start_element_loop(element_loops[1], hdri[1], hdro);
            blob element_buf(checked_cast<size_t>(hdro.element_size()));
            std::vector<size_t> component_offsets(hdro.components());
            for (uintmax_t e = 0; e < hdro.elements(); e++)
            {
                for (uintmax_t c = 0; c < hdro.components(); c++)
                {
                    component_offsets[c] = static_cast<const char*>(hdro.component(element_buf.ptr(), c))
                        - element_buf.ptr<const char>();
                }
            }
            for (uintmax_t e = 0; e < hdro.elements(); e++)
            {
                const void* e0 = element_loops[0].read();
                const void* e1 = element_loops[1].read();
                for (uintmax_t c = 0; c < hdro.components(); c++)
                {
                    diff(hdro.component_type(c), absolute.value(), force.value(),
                            static_cast<const void*>(static_cast<const char*>(e0) + component_offsets[c]),
                            static_cast<const void*>(static_cast<const char*>(e1) + component_offsets[c]),
                            static_cast<void*>(element_buf.ptr<char>(component_offsets[c])));
                }
                element_loops[0].write(element_buf.ptr());
            }
        }
        array_loops[0].finish();
        if (array_loops[1].read(hdri[1], namei[1]))
        {
            msg::wrn_txt("ignoring additional array(s) from %s", arguments[1].c_str());
        }
        array_loops[1].finish();
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
