/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010, 2011, 2012, 2013
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
#include <cctype>

#include <gta/gta.hpp>

#include "base/msg.h"
#include "base/blb.h"
#include "base/opt.h"
#include "base/str.h"
#include "base/chk.h"

#include "lib.h"


extern "C" void gtatool_dimension_extract_help(void)
{
    msg::req_txt(
            "dimension-extract [-d|--dimension=<d>] [-i|--index=<i>] [<files>...]\n"
            "\n"
            "Reduces the dimensionality of the input GTAs by extracting the given index of the given dimension "
            "and removing that dimension. For example, you can extract a 2D slice from a 3D volume, or a "
            "1D line from a 2D image. (To extract array subsets while keeping the number of dimensions, use "
            "the extract command). By default, index 0 from the highest dimension is extracted.\n"
            "Example: dimension-extract -d 1 -i 128 volume.gta > y-slice.gta");
}

extern "C" int gtatool_dimension_extract(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::val<uintmax_t> dimension("dimension", 'd', opt::optional);
    options.push_back(&dimension);
    opt::val<uintmax_t> index("index", 'i', opt::optional);
    options.push_back(&index);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_dimension_extract_help();
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
            if (hdri.dimensions() == 0)
            {
                throw exc(namei + ": array has zero dimensions");
            }
            uintmax_t dim = (dimension.values().empty() ? hdri.dimensions() - 1 : dimension.value());
            if (dim >= hdri.dimensions())
            {
                throw exc(namei + ": array has no dimension " + str::from(dim));
            }
            uintmax_t ind = (index.values().empty() ? hdri.dimension_size(dim) - 1: index.value());
            if (ind >= hdri.dimension_size(dim))
            {
                throw exc(namei + ": array dimension " + str::from(dim) + " has no index " + str::from(ind));
            }
            std::vector<uintmax_t> dim_sizes;
            for (uintmax_t i = 0; i < hdri.dimensions(); i++)
            {
                if (i != dim)
                {
                    dim_sizes.push_back(hdri.dimension_size(i));
                }
            }
            hdro = hdri;
            hdro.set_compression(gta::none);
            hdro.set_dimensions(dim_sizes.size(), dim_sizes.size() > 0 ? &(dim_sizes[0]) : NULL);
            uintmax_t hdro_dim = 0;
            for (uintmax_t i = 0; i < hdri.dimensions(); i++)
            {
                if (i != dim)
                {
                    hdro.dimension_taglist(hdro_dim++) = hdri.dimension_taglist(i);
                }
            }
            array_loop.write(hdro, nameo);
            if (hdro.data_size() > 0)
            {
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, hdri, hdro);
                std::vector<uintmax_t> indices(hdri.dimensions());
                for (uintmax_t i = 0; i < hdri.elements(); i++)
                {
                    const void *e = element_loop.read();
                    hdri.linear_index_to_indices(i, &(indices[0]));
                    if (indices[dim] == ind)
                    {
                        element_loop.write(e);
                    }
                }
            }
            else
            {
                array_loop.skip_data(hdri);
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
