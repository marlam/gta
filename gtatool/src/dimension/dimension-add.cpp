/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010, 2011, 2013
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
#include <cstdio>
#include <cstring>

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "opt.h"
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

    try
    {
        array_loop_t array_loop;
        gta::header hdri, hdro;
        std::string namei, nameo;
        array_loop.start(arguments, "");
        while (array_loop.read(hdri, namei))
        {
            uintmax_t dim = (dimension.values().empty() ? hdri.dimensions() : dimension.value());
            if (dim > hdri.dimensions())
            {
                throw exc(namei + ": cannot add dimension " + str::from(dim));
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
            array_loop.write(hdro, nameo);
            // Write the GTA data
            if (hdro.data_size() > 0)
            {
                if (hdri.data_size() == 0)
                {
                    // We had an empty (dimensionless) array before, and now have
                    // one with exactly one element. Create the data.
                    blob element(checked_cast<size_t>(hdro.element_size()));
                    std::memset(element.ptr(), 0, hdro.element_size());
                    array_loop.write_data(hdro, element.ptr());
                }
                else
                {
                    element_loop_t element_loop;
                    array_loop.start_element_loop(element_loop, hdri, hdro);
                    for (uintmax_t i = 0; i < hdri.elements(); i++)
                    {
                        element_loop.write(element_loop.read());
                    }
                }
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
