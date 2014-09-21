/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2012, 2013, 2014
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

#include <string>
#include <limits>
#include <cstdio>
#include <cstring>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <gta/gta.hpp>

#include "base/msg.h"
#include "base/str.h"
#include "base/fio.h"
#include "base/opt.h"

#include "lib.h"

#include "delimiter.h"


extern "C" void gtatool_to_csv_help(void)
{
    msg::req_txt("to-csv [-D|--delimiter=D] [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to csv format, using the field delimiter D. "
            "D is a single ASCII character; the default is the comma (',').\n"
            "If more than one array is available in the input, the arrays will "
            "be separated by blank lines in the output.");
}

static void write_component(const void* c, gta::type t, char s[32])
{
    if (t == gta::int8)
    {
        int8_t v;
        std::memcpy(&v, c, sizeof(int8_t));
        snprintf(s, 32, "%" PRId8, v);
    }
    else if (t == gta::uint8)
    {
        uint8_t v;
        std::memcpy(&v, c, sizeof(uint8_t));
        snprintf(s, 32, "%" PRIu8, v);
    }
    else if (t == gta::int16)
    {
        int16_t v;
        std::memcpy(&v, c, sizeof(int16_t));
        snprintf(s, 32, "%" PRId16, v);
    }
    else if (t == gta::uint16)
    {
        uint16_t v;
        std::memcpy(&v, c, sizeof(uint16_t));
        snprintf(s, 32, "%" PRIu16, v);
    }
    else if (t == gta::int32)
    {
        int32_t v;
        std::memcpy(&v, c, sizeof(int32_t));
        snprintf(s, 32, "%" PRId32, v);
    }
    else if (t == gta::uint32)
    {
        uint32_t v;
        std::memcpy(&v, c, sizeof(uint32_t));
        snprintf(s, 32, "%" PRIu32, v);
    }
    else if (t == gta::int64)
    {
        int64_t v;
        std::memcpy(&v, c, sizeof(int64_t));
        snprintf(s, 32, "%" PRId64, v);
    }
    else if (t == gta::uint64)
    {
        uint64_t v;
        std::memcpy(&v, c, sizeof(uint64_t));
        snprintf(s, 32, "%" PRIu64, v);
    }
    else if (t == gta::float32)
    {
        float v;
        std::memcpy(&v, c, sizeof(float));
        snprintf(s, 32, "%.9g", v);
    }
    else // t == gta::float64
    {
        double v;
        std::memcpy(&v, c, sizeof(double));
        snprintf(s, 32, "%.17g", v);
    }
}

extern "C" int gtatool_to_csv(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> delimiters = gta_csv_create_delimiters();
    opt::string delimiter("delimiter", 'D', opt::optional, delimiters, std::string(","));
    options.push_back(&delimiter);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, 2, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_to_csv_help();
        return 0;
    }

    try
    {
        std::string nameo = arguments.size() == 1 ? arguments[0] : arguments[1];
        FILE *fo = fio::open(nameo, "w");

        array_loop_t array_loop;
        gta::header hdr;
        std::string name;

        array_loop.start(arguments.size() == 1 ? std::vector<std::string>() : std::vector<std::string>(1, arguments[0]), nameo);
        while (array_loop.read(hdr, name))
        {
            if (hdr.dimensions() != 1 && hdr.dimensions() != 2)
            {
                throw exc(name + ": only one- or two-dimensional arrays can be converted to CSV.");
            }
            if (hdr.components() < 1)
            {
                throw exc(name + ": unsupported number of element components.");
            }
            for (uintmax_t c = 0; c < hdr.components(); c++)
            {
                if (hdr.component_type(c) != gta::int8
                        && hdr.component_type(c) != gta::uint8
                        && hdr.component_type(c) != gta::int16
                        && hdr.component_type(c) != gta::uint16
                        && hdr.component_type(c) != gta::int32
                        && hdr.component_type(c) != gta::uint32
                        && hdr.component_type(c) != gta::int64
                        && hdr.component_type(c) != gta::uint64
                        && hdr.component_type(c) != gta::float32
                        && hdr.component_type(c) != gta::float64)
                {
                    throw exc(name + ": unsupported element component type(s).");
                }
            }
            if (array_loop.index_in() > 1)
            {
                if (std::fputs("\r\n", fo) == EOF)
                {
                    throw exc(nameo + ": output error.");
                }
            }

            std::vector<blob> no_data_values(checked_cast<size_t>(hdr.components()));
            for (uintmax_t c = 0; c < hdr.components(); c++)
            {
                const char* tagval = hdr.component_taglist(c).get("NO_DATA_VALUE");
                if (tagval)
                {
                    no_data_values[c].resize(hdr.component_size(c));
                    try
                    {
                        value_from_string(std::string(tagval),
                                hdr.component_type(c), hdr.component_size(c),
                                no_data_values[c].ptr());
                    }
                    catch (std::exception& e)
                    {
                        msg::wrn(name + ": component " + str::from(c) + ": invalid NO_DATA_VALUE");
                        no_data_values[c].free();
                    }
                }
            }

            char sbuf[32];
            element_loop_t element_loop;
            array_loop.start_element_loop(element_loop, hdr, gta::header());
            for (uintmax_t e = 0; e < hdr.elements(); e++)
            {
                const void *p = element_loop.read();
                for (uintmax_t c = 0; c < hdr.components(); c++)
                {
                    if (no_data_values[c].size() != 0
                            && std::memcmp(no_data_values[c].ptr(), hdr.component(p, c), no_data_values[c].size()) == 0)
                    {
                        sbuf[0] = '\0';
                    }
                    else
                    {
                        write_component(hdr.component(p, c), hdr.component_type(c), sbuf);
                    }
                    if (std::fputs(sbuf, fo) == EOF)
                    {
                        throw exc(nameo + ": output error.");
                    }
                    if (c < hdr.components() - 1 && std::fputs(delimiter.value().c_str(), fo) == EOF)
                    {
                        throw exc(nameo + ": output error.");
                    }
                }
                if (e == hdr.elements() - 1 ||
                        (hdr.dimensions() == 2 && e % hdr.dimension_size(0) == hdr.dimension_size(0) - 1))
                {
                    if (std::fputs("\r\n", fo) == EOF)
                    {
                        throw exc(nameo + ": output error.");
                    }
                }
                else
                {
                    if (std::fputs(delimiter.value().c_str(), fo) == EOF)
                    {
                        throw exc(nameo + ": output error.");
                    }
                }
            }
        }
        fio::flush(fo, nameo);
        if (std::ferror(fo))
        {
            throw exc(nameo + ": output error.");
        }
        fio::close(fo, nameo);
        array_loop.finish();
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
