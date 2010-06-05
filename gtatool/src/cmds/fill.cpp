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

    if (cio::isatty(gtatool_stdout))
    {
        msg::err_txt("refusing to write to a tty");
        return 1;
    }

    try
    {
        gta::header hdri;
        gta::header hdro;
        // Loop over all input files
        size_t arg = 0;
        do
        {
            std::string finame = (arguments.size() == 0 ? "standard input" : arguments[arg]);
            FILE *fi = (arguments.size() == 0 ? gtatool_stdin : cio::open(finame, "r"));

            // Loop over all GTAs inside the current file
            uintmax_t array_index = 0;
            while (cio::has_more(fi, finame))
            {
                // Determine the name of the array for error messages
                std::string array_name = finame + " array " + str::from(array_index);
                // Read the GTA header
                hdri.read_from(fi);
                // Determine compatibility and read value
                if (!low.values().empty() && low.value().size() != hdri.dimensions())
                {
                    throw exc(array_name + ": array has incompatible number of dimensions");
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
                // Write the GTA header
                hdro = hdri;
                hdro.set_compression(gta::none);
                hdro.write_to(gtatool_stdout);
                // Manipulate the GTA data
                std::vector<uintmax_t> index(hdri.dimensions());
                blob element(checked_cast<size_t>(hdro.element_size()));
                gta::io_state si, so;
                for (uintmax_t e = 0; e < hdro.elements(); e++)
                {
                    hdri.read_elements(si, fi, 1, element.ptr());
                    linear_index_to_indices(hdri, e, &(index[0]));
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
                    if (replace)
                    {
                        memcpy(element.ptr(), v.ptr(), element.size());
                    }
                    hdro.write_elements(so, gtatool_stdout, 1, element.ptr());
                }
                array_index++;
            }
            if (fi != gtatool_stdin)
            {
                cio::close(fi);
            }
            arg++;
        }
        while (arg < arguments.size());
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
