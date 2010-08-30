/*
 * fill.cpp
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
#include <cctype>

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "opt.h"
#include "cio.h"
#include "str.h"
#include "intcheck.h"

#include "lib.h"


extern "C" void gtatool_fill_help(void)
{
    msg::req_txt(
            "fill [-l|--low=<l0>[,<l1>[,...]]] [-h|--high=<h0>[,<h1>[,...]]] [-v|--value=<v0>[,<v1>[,...]]] [<files>...]\n"
            "\n"
            "Fills a subset of the input arrays with a given value. The subset is given by its low and high coordinates (inclusive). "
            "The default is to fill the complete array with zeroes.\n"
            "Example: fill -l 20,20 -h 29,29 -v 32,64,128 < img1.gta > img2.gta");
}

extern "C" int gtatool_fill(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::tuple<uintmax_t> low("low", 'l', opt::optional);
    options.push_back(&low);
    opt::tuple<uintmax_t> high("high", 'h', opt::optional);
    options.push_back(&high);
    opt::string value("value", 'v', opt::optional);
    options.push_back(&value);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_fill_help();
        return 0;
    }
    if ((low.values().empty() && !high.values().empty()) || (!low.values().empty() && high.values().empty()))
    {
        msg::err_txt("must specify none or both of low and high coordinates");
        return 1;
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
            if (!low.values().empty() && low.value().size() != hdri.dimensions())
            {
                throw exc(namei + ": array has incompatible number of dimensions");
            }
            blob v(checked_cast<size_t>(hdri.element_size()));
            if (value.values().empty())
            {
                memset(v.ptr(), 0, hdri.element_size());
            }
            else
            {
                std::vector<gta::type> comp_types;
                std::vector<uintmax_t> comp_sizes;
                for (uintmax_t i = 0; i < hdri.components(); i++)
                {
                    comp_types.push_back(hdri.component_type(i));
                    if (hdri.component_type(i) == gta::blob)
                    {
                        comp_sizes.push_back(hdri.component_size(i));
                    }
                }
                valuelist_from_string(value.value(), comp_types, comp_sizes, v.ptr());
            }
            hdro = hdri;
            hdro.set_compression(gta::none);
            array_loop.write(hdro, nameo);

            element_loop_t element_loop;
            std::vector<uintmax_t> index(hdri.dimensions());
            array_loop.start_element_loop(element_loop, hdri, hdro);
            for (uintmax_t e = 0; e < hdro.elements(); e++)
            {
                const void *src = element_loop.read();
                hdro.linear_index_to_indices(e, &(index[0]));
                bool replace = true;
                if (!low.values().empty())
                {
                    for (size_t i = 0; i < index.size(); i++)
                    {
                        if (index[i] < low.value()[i] || index[i] > high.value()[i])
                        {
                            replace = false;
                            break;
                        }
                    }
                }
                element_loop.write(replace ? v.ptr() : src);
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
