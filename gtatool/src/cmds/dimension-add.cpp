/*
 * dimension-add.cpp
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


extern "C" void gtatool_dimension_add_help(void)
{
    msg::req_txt(
            "dimension-add [-d|--dimension=<d>] [<files>...]\n"
            "\n"
            "Increases the dimensionality of the input GTAs by one by adding an additional dimension of size 1 "
            "at the given dimension index d. The default is to append the new dimension. "
            "Note that the data of the array remains unchanged.\n"
            "Example: dimension-add slice.gta > thin-volume.gta");
}

extern "C" int gtatool_dimension_add(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::val<uintmax_t> dimension("dimension", 'd', opt::optional);
    options.push_back(&dimension);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_dimension_add_help();
        return 0;
    }

    if (cio::isatty(stdout))
    {
        msg::err("refusing to write to a tty");
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
            FILE *fi = (arguments.size() == 0 ? stdin : cio::open(finame, "r"));

            // Loop over all GTAs inside the current file
            uintmax_t array_index = 0;
            while (cio::has_more(fi, finame))
            {
                // Determine the name of the array for error messages
                std::string array_name = finame + " array " + str::from(array_index);
                // Read the GTA header
                hdri.read_from(fi);
                uintmax_t dim = (dimension.values().empty() ? hdri.dimensions() : dimension.value());
                if (dim > hdri.dimensions())
                {
                    throw exc(array_name + ": cannot add dimension " + str::from(dim));
                }
                // Determine the new dimensions
                std::vector<uintmax_t> dim_sizes;
                for (uintmax_t i = 0; i < hdri.dimensions(); i++)
                {
                    if (i == dim)
                    {
                        dim_sizes.push_back(1);
                    }
                    dim_sizes.push_back(hdri.dimension_size(i));
                }
                if (dim == hdri.dimensions())
                {
                    dim_sizes.push_back(1);
                }
                hdro = hdri;
                hdro.set_compression(gta::none);
                hdro.set_dimensions(dim_sizes.size(), &(dim_sizes[0]));
                uintmax_t hdro_dim = 0;
                for (uintmax_t i = 0; i < hdri.dimensions(); i++)
                {
                    if (i == dim)
                    {
                        hdro_dim++;
                    }
                    hdro.dimension_taglist(hdro_dim++) = hdri.dimension_taglist(i);
                }
                // Write the GTA header
                hdro.write_to(stdout);
                // Manipulate the GTA data
                blob element(checked_cast<size_t>(hdri.element_size()));
                gta::io_state si, so;
                for (uintmax_t e = 0; e < hdri.elements(); e++)
                {
                    hdri.read_elements(si, fi, 1, element.ptr());
                    hdro.write_elements(so, stdout, 1, element.ptr());
                }
                array_index++;
            }
            if (fi != stdin)
            {
                cio::close(fi);
            }
            arg++;
        }
        while (arg < arguments.size());
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        return 1;
    }

    return 0;
}
