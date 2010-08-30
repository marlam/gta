/*
 * stream-extract.cpp
 *
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010
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
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cctype>

#include <gta/gta.hpp>

#include "msg.h"
#include "opt.h"
#include "cio.h"
#include "str.h"

#include "lib.h"


extern "C" void gtatool_stream_extract_help(void)
{
    msg::req_txt(
            "stream-extract [-d|--drop] <range1>[,<range2>[,...]] [<files...>]\n"
            "\n"
            "Selects arrays from the input stream of arrays and writes them to standard output. "
            "Other arrays are discarded.\n"
            "Arrays can be selected using a list of one or more range descriptions of the form a-b (to select "
            "input arrays a through b), a- (to select all arrays starting with a), -b (to select "
            "all arrays up to and including b), or b (to select the single array b).\n"
            "If --drop is used, the selection is inverted: the selected arrays are discarded and all others "
            "written to standard output.\n"
            "Example:\n"
            "stream-extract 1-3,9-15 many-arrays.gta > subset.gta");
}

/* The code to hangle range lists was partially taken from cvtool-1.0.0 on 2010-05-14. */

typedef struct
{
    uintmax_t a;
    uintmax_t b;
} range_t;

static range_t to_range(const std::string &s)
{
    range_t r;
    size_t minus = s.find_first_of('-');
    if (minus == std::string::npos)     // 'b'
    {
        r.a = str::to<uintmax_t>(s);
        r.b = r.a;
    }
    else if (minus == 0)                // '-b'
    {
        r.a = 0;
        r.b = str::to<uintmax_t>(s.substr(1));
    }
    else if (minus == s.length() - 1)   // 'a-'
    {
        r.a = str::to<uintmax_t>(s.substr(0, s.length() - 1));
        r.b = std::numeric_limits<uintmax_t>::max();
    }
    else                                // 'a-b'
    {
        r.a = str::to<uintmax_t>(s.substr(0, minus));
        r.b = str::to<uintmax_t>(s.substr(minus + 1));
    }
    return r;
}

static std::vector<range_t> to_rangelist(const std::string &s)
{
    std::vector<range_t> list;
    size_t from = 0;
    size_t to;
    do
    {
        to = s.find_first_of(',', from);
        list.push_back(to_range(s.substr(from, to == std::string::npos ? to : to - from)));
        from = to + 1;
    }
    while (to != std::string::npos);
    return list;
}

static bool cmp_ranges(range_t r1, range_t r2)
{
    return (r1.a < r2.a);
}

/* Merges overlapping ranges and sorts them in ascending order of their start values. */
static void normalize(std::vector<range_t> &rangelist)
{
    std::sort(rangelist.begin(), rangelist.end(), cmp_ranges);

    if (rangelist.size() > 1)
    {
        for (size_t i = 0; i < rangelist.size() - 1; /* increment is done below */)
        {
            if (rangelist[i + 1].a == 0 || (rangelist[i + 1].a - 1 <= rangelist[i].b))
            {
                if (rangelist[i + 1].b > rangelist[i].b)
                {
                    rangelist[i].b = rangelist[i + 1].b;
                }
                rangelist.erase(rangelist.begin() + i + 1);
            }
            else
            {
                i++;
            }
        }
    }
}

/* Checks whether the given index is in one of the ranges in the range list.
 * The range list must be normalized. 'ranges_index' must be zero on the first call.
 * Subsequent calls must have increasing values of 'index'. */
static bool in_range(const std::vector<range_t> &rangelist, size_t *ranges_index, uintmax_t index)
{
    /* test all frame ranges beginning at ranges_index */
    for (size_t i = *ranges_index; i < rangelist.size(); i++)
    {
        if (index > rangelist[i].b)
        {
            /* this frame range cannot be reached anymore */
            *ranges_index = i + 1;
            continue;
        }
        else if (index >= rangelist[i].a)
        {
            /* index is in the frame range i */
            return true;
        }
        else
        {
            /* index < ranges[i].a. Since the following frame ranges have start
             * values >= ranges[i].a, we don't need to test them */
            break;
        }
    }
    return false;
}


extern "C" int gtatool_stream_extract(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::flag drop("drop", 'd', opt::optional);
    options.push_back(&drop);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_stream_extract_help();
        return 0;
    }

    std::vector<range_t> rangelist;
    try
    {
        rangelist = to_rangelist(arguments.front());
        normalize(rangelist);
        arguments.erase(arguments.begin());
    }
    catch (...)
    {
        msg::err_txt("invalid range list");
        return 1;
    }

    try
    {
        array_loop_t array_loop;
        gta::header hdri, hdro;
        std::string namei, nameo;
        uintmax_t array_index = 0;
        size_t rangelist_index = 0;
        uintmax_t dropcounter = 0;
        array_loop.start(arguments, "");
        while (array_loop.read(hdri, namei))
        {
            bool keep = in_range(rangelist, &rangelist_index, array_index);
            if (drop.value())
            {
                keep = !keep;
            }
            if (keep)
            {
                hdro = hdri;
                hdro.set_compression(gta::none);
                array_loop.write(hdro, nameo);
                array_loop.copy_data(hdri, hdro);
            }
            else
            {
                array_loop.skip_data(hdri);
                dropcounter++;
            }
            array_index++;
        }
        array_loop.finish();
        msg::dbg_txt(str::from(array_index) + " arrays processed, "
                + str::from(array_index - dropcounter) + " kept, "
                + str::from(dropcounter) + " dropped");
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
