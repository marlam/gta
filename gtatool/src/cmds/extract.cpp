/*
 * extract.cpp
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

#include "config.h"

#include <sstream>
#include <cstdio>
#include <cstring>
#include <cctype>

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "opt.h"
#include "cio.h"
#include "str.h"
#include "intcheck.h"

#include "lib.h"


extern "C" void gtatool_extract_help(void)
{
    msg::req_txt(
            "extract -l|--low=<l0>[,<l1>[,...]] -h|--high=<h0>[,<h1>[,...]] [<files>...]\n"
            "\n"
            "Extracts a sub-array from each input GTA and writes it to standard output. "
            "The sub-array is given by its lower and higher coordinates (inclusive).\n"
            "Example: extract -l 10,10 -h 19,19 image.gta > image-10x10.gta");
}

extern "C" int gtatool_extract(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::tuple<uintmax_t> low("low", 'l', opt::required);
    options.push_back(&low);
    opt::tuple<uintmax_t> high("high", 'h', opt::required);
    options.push_back(&high);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_extract_help();
        return 0;
    }
    if (low.value().size() != high.value().size())
    {
        msg::err_txt("low and high coordinates must have the same dimensions");
        return 1;
    }
    for (size_t i = 0; i < low.value().size(); i++)
    {
        if (low.value()[i] > high.value()[i])
        {
            msg::err_txt("low coordinate(s) are greater than high coordinate(s)");
            return 1;
        }
    }

    try
    {
        array_loop_t array_loop;
        gta::header hdri, hdro;
        std::string namei, nameo;
        array_loop.start(arguments, "");
        while (array_loop.read(hdri, namei))
        {
            if (hdri.dimensions() != low.value().size())
            {
                throw exc(namei + ": array has " + str::from(hdri.dimensions())
                        + " dimensions, but sub-array has " + str::from(low.value().size()));
            }
            for (uintmax_t i = 0; i < hdri.dimensions(); i++)
            {
                if (hdri.dimension_size(i) <= high.value()[i])
                {
                    throw exc(namei + ": array does not contain the requested sub-array");
                }
            }
            std::vector<uintmax_t> dim_sizes(low.value().size());
            for (uintmax_t i = 0; i < hdri.dimensions(); i++)
            {
                dim_sizes[i] = high.value()[i] - low.value()[i] + 1;
            }
            hdro = hdri;
            hdro.set_compression(gta::none);
            hdro.set_dimensions(dim_sizes.size(), dim_sizes.size() > 0 ? &(dim_sizes[0]) : NULL);
            for (uintmax_t i = 0; i < hdri.dimensions(); i++)
            {
                hdro.dimension_taglist(i) = hdri.dimension_taglist(i);
            }
            array_loop.write(hdro, nameo);

            element_loop_t element_loop;
            std::vector<uintmax_t> index(hdro.dimensions());
            array_loop.start_element_loop(element_loop, hdri, hdro);
            for (uintmax_t e = 0; e < hdri.elements(); e++)
            {
                const void *src = element_loop.read();
                hdri.linear_index_to_indices(e, &(index[0]));
                bool in_sub_array = true;
                for (size_t i = 0; i < index.size(); i++)
                {
                    if (index[i] < low.value()[i] || index[i] > high.value()[i])
                    {
                        in_sub_array = false;
                        break;
                    }
                }
                if (in_sub_array)
                {
                    element_loop.write(src);
                }
            }
            element_loop.finish();
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
