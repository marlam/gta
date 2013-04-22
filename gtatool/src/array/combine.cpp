/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2013
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

#include <vector>

#include <gta/gta.hpp>

#include "msg.h"
#include "opt.h"
#include "fio.h"
#include "intcheck.h"

#include "lib.h"


extern "C" void gtatool_combine_help(void)
{
    msg::req_txt(
            "combine -m|--mode=min|max|add|sub|mul|div|or|and|xor [-f|--force] <files>...\n"
            "\n"
            "Combines the input GTAs in the given mode and writes the result to stdout.\n"
            "The GTAs must be compatible in dimensions and component types. This command produces "
            "output GTAs of the same kind. Each component will contain the result of combining "
            "the corresponding components in the input GTAs.\n"
            "Beware of limitations of the integer type range! If a difference cannot be represented "
            "in the given component type (e.g. 10 - 20 in uint8), this command will abort by default. "
            "Use -f to force clamping of values to the representable range instead, or use the "
            "component-convert command to work with different component types.\n"
            "Example: combine -m min a.gta b.gta > min.gta");
}

typedef enum
{
    mode_min, mode_max, mode_add, mode_sub, mode_mul, mode_div, mode_and, mode_or, mode_xor
} combine_mode_t;

template<typename T>
static void int_arith_combine(combine_mode_t mode, bool force, size_t n, const void** c, void* l)
{
    T r;
    std::memcpy(&r, c[0], sizeof(T));
    try
    {
        for (size_t i = 1; i < n; i++)
        {
            T s;
            std::memcpy(&s, c[i], sizeof(T));
            switch (mode)
            {
            case mode_min:
                if (s < r)
                    r = s;
                break;
            case mode_max:
                if (s > r)
                    r = s;
                break;
            case mode_add:
                r = checked_add(r, s);
                break;
            case mode_sub:
                r = checked_sub(r, s);
                break;
            case mode_mul:
                r = checked_mul(r, s);
                break;
            case mode_div:
                r = checked_div(r, s);
                break;
            default:
                // cannot happen
                break;
            }
        }
    }
    catch (std::overflow_error&) {
        if (force)
            r = std::numeric_limits<T>::max();
        else
            throw;
    }
    catch (std::underflow_error&) {
        if (force)
            r = std::numeric_limits<T>::min();
        else
            throw;
    }
    std::memcpy(l, &r, sizeof(T));
}

template<typename T>
static void float_arith_combine(combine_mode_t mode, size_t n, const void** c, void* l)
{
    T r;
    std::memcpy(&r, c[0], sizeof(T));
    for (size_t i = 1; i < n; i++)
    {
        T s;
        std::memcpy(&s, c[i], sizeof(T));
        switch (mode)
        {
        case mode_min:
            if (s < r)
                r = s;
            break;
        case mode_max:
            if (s > r)
                r = s;
            break;
        case mode_add:
            r += s;
            break;
        case mode_sub:
            r -= s;
            break;
        case mode_mul:
            r *= s;
            break;
        case mode_div:
            r /= s;
            break;
        default:
            // cannot happen
            break;
        }
    }
    std::memcpy(l, &r, sizeof(T));
}

template<typename T>
static void bit_combine(combine_mode_t mode, size_t n, const void** c, void* l)
{
    T r;
    std::memcpy(&r, c[0], sizeof(T));
    for (size_t i = 1; i < n; i++)
    {
        T s;
        std::memcpy(&s, c[i], sizeof(T));
        switch (mode)
        {
        case mode_or:
            r |= s;
            break;
        case mode_and:
            r &= s;
            break;
        case mode_xor:
            r ^= s;
            break;
        default:
            // cannot happen
            break;
        }
    }
    std::memcpy(l, &r, sizeof(T));
}

static void combine(gta::type t, combine_mode_t mode, bool force, size_t n, const void** c, void* l)
{
    if (mode == mode_and || mode == mode_or || mode == mode_xor)
    {
        if (t == gta::int8 || t == gta::uint8)
            bit_combine<uint8_t>(mode, n, c, l);
        else if (t == gta::int16 || t == gta::uint16)
            bit_combine<uint16_t>(mode, n, c, l);
        else if (t == gta::int32 || t == gta::uint32 || t == gta::float32)
            bit_combine<uint32_t>(mode, n, c, l);
        else // (t == gta::int64 || t == gta::uint64 || t == gta::float64)
            bit_combine<uint64_t>(mode, n, c, l);
    }
    else
    {
        if (t == gta::int8)
            int_arith_combine<int8_t>(mode, force, n, c, l);
        else if (t == gta::uint8)
            int_arith_combine<uint8_t>(mode, force, n, c, l);
        else if (t == gta::int16)
            int_arith_combine<int16_t>(mode, force, n, c, l);
        else if (t == gta::uint16)
            int_arith_combine<uint16_t>(mode, force, n, c, l);
        else if (t == gta::int32)
            int_arith_combine<int32_t>(mode, force, n, c, l);
        else if (t == gta::uint32)
            int_arith_combine<uint32_t>(mode, force, n, c, l);
        else if (t == gta::int64)
            int_arith_combine<int64_t>(mode, force, n, c, l);
        else if (t == gta::uint64)
            int_arith_combine<uint64_t>(mode, force, n, c, l);
        else if (t == gta::float32)
            float_arith_combine<float>(mode, n, c, l);
        else // t == gta::float64
            float_arith_combine<double>(mode, n, c, l);
    }
}

extern "C" int gtatool_combine(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> modes;
    modes.push_back("min");
    modes.push_back("max");
    modes.push_back("add");
    modes.push_back("sub");
    modes.push_back("mul");
    modes.push_back("div");
    modes.push_back("and");
    modes.push_back("or");
    modes.push_back("xor");
    opt::val<std::string> mode("mode", 'm', opt::required, modes);
    options.push_back(&mode);
    opt::flag force("force", 'f', opt::optional);
    options.push_back(&force);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_combine_help();
        return 0;
    }

    combine_mode_t m
        (  mode.value().compare("min") == 0 ? mode_min
         : mode.value().compare("max") == 0 ? mode_max
         : mode.value().compare("add") == 0 ? mode_add
         : mode.value().compare("sub") == 0 ? mode_sub
         : mode.value().compare("mul") == 0 ? mode_mul
         : mode.value().compare("div") == 0 ? mode_div
         : mode.value().compare("and") == 0 ? mode_and
         : mode.value().compare("or") == 0 ? mode_or
         : mode_xor);

    try
    {
        std::vector<array_loop_t> array_loops(arguments.size());
        std::vector<gta::header> hdri(arguments.size());
        std::vector<std::string> namei(arguments.size());

        for (size_t i = 0; i < arguments.size(); i++)
        {
            array_loops[i].start(arguments[i], "");
        }
        while (array_loops[0].read(hdri[0], namei[0]))
        {
            bool have_input = true;
            for (size_t i = 1; i < arguments.size(); i++)
            {
                if (!array_loops[i].read(hdri[i], namei[i]))
                {
                    have_input = false;
                    break;
                }
                if (hdri[i].components() != hdri[0].components())
                {
                    throw exc(namei[i] + ": incompatible array");
                }
                for (uintmax_t c = 0; c < hdri[0].components(); c++)
                {
                    if (hdri[i].component_type(c) != hdri[0].component_type(c)
                            || hdri[i].component_size(c) != hdri[0].component_size(c))
                    {
                        throw exc(namei[i] + ": incompatible array");
                    }
                    if (hdri[i].component_type(c) != gta::int8
                            && hdri[i].component_type(c) != gta::uint8
                            && hdri[i].component_type(c) != gta::int16
                            && hdri[i].component_type(c) != gta::uint16
                            && hdri[i].component_type(c) != gta::int32
                            && hdri[i].component_type(c) != gta::uint32
                            && hdri[i].component_type(c) != gta::int64
                            && hdri[i].component_type(c) != gta::uint64
                            && hdri[i].component_type(c) != gta::float32
                            && hdri[i].component_type(c) != gta::float64)
                    {
                        throw exc(namei[i] + ": cannot compute combinations of type "
                                + type_to_string(hdri[i].component_type(c), hdri[i].component_size(c)));
                    }
                }
                if (hdri[i].dimensions() != hdri[0].dimensions())
                {
                    throw exc(namei[i] + ": incompatible array");
                }
                for (uintmax_t d = 0; d < hdri[0].dimensions(); d++)
                {
                    if (hdri[i].dimension_size(d) != hdri[0].dimension_size(d))
                    {
                        throw exc(namei[i] + ": incompatible array");
                    }
                }
            }
            if (!have_input)
            {
                msg::wrn_txt("ignoring additional array(s) from %s", arguments[0].c_str());
                break;
            }

            gta::header hdro = hdri[0];
            hdro.set_compression(gta::none);
            std::string nameo;
            array_loops[0].write(hdro, nameo);
            if (hdro.data_size() == 0)
            {
                continue;
            }

            std::vector<element_loop_t> element_loops(arguments.size());
            for (size_t i = 0; i < arguments.size(); i++)
            {
                array_loops[i].start_element_loop(element_loops[i], hdri[i], hdro);
            }
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
            std::vector<const void*> element_ptrs(arguments.size());
            std::vector<const void*> component_ptrs(arguments.size());
            for (uintmax_t e = 0; e < hdro.elements(); e++)
            {
                for (size_t i = 0; i < arguments.size(); i++)
                {
                    element_ptrs[i] = element_loops[i].read();
                }
                for (uintmax_t c = 0; c < hdro.components(); c++)
                {
                    for (size_t i = 0; i < arguments.size(); i++)
                    {
                        component_ptrs[i] = static_cast<const char*>(element_ptrs[i]) + component_offsets[c];
                    }
                    combine(hdro.component_type(c), m, force.value(), arguments.size(), &component_ptrs[0],
                            static_cast<void*>(element_buf.ptr<char>(component_offsets[c])));
                }
                element_loops[0].write(element_buf.ptr());
            }
        }
        array_loops[0].finish();
        for (size_t i = 1; i < arguments.size(); i++)
        {
            if (array_loops[i].read(hdri[i], namei[i]))
            {
                msg::wrn_txt("ignoring additional array(s) from %s", arguments[i].c_str());
            }
            array_loops[i].finish();
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
