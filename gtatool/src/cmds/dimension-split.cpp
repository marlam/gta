/*
 * dimension-split.cpp
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


extern "C" void gtatool_dimension_split_help(void)
{
    msg::req_txt(
            "dimension-split [-d|--dimension=<d>] [<files>...]\n"
            "\n"
            "Reduces the dimensionality of the input GTAs by splitting them at the given dimension.\n"
            "For example, a 13x17x19 volume can be split at dimension 2 into 19 2D images of size 13x17, or "
            "at dimension 0 into 13 images of size 17x19.\n"
            "By default, the GTAs are split at the highest dimension.\n"
            "If you only want to extract specific indices of a dimension, use the dimension-extract command instead.\n"
            "All output arrays will be written into a single stream; if you want separate files, "
            "pipe this stream through the stream-split command.\n"
            "Example: dimension-split volume.gta > slices.gta");
}

extern "C" int gtatool_dimension_split(int argc, char *argv[])
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
        gtatool_dimension_split_help();
        return 0;
    }

    if (cio::isatty(stdout))
    {
        msg::err_txt("refusing to write to a tty");
        return 1;
    }

    try
    {
        gta::header hdri;
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
                uintmax_t dim = (dimension.values().empty() ? hdri.dimensions() - 1 : dimension.value());
                if (dim >= hdri.dimensions())
                {
                    throw exc(array_name + ": array has no dimension " + str::from(dim));
                }
                // Determine the new dimensions
                std::vector<uintmax_t> dim_sizes;
                for (uintmax_t i = 0; i < hdri.dimensions(); i++)
                {
                    if (i != dim)
                    {
                        dim_sizes.push_back(hdri.dimension_size(i));
                    }
                }
                // Define the GTA headers and create temporary files
                std::vector<gta::header> hdros(checked_cast<size_t>(hdri.dimension_size(dim)));
                std::vector<FILE *> tmpfiles(hdros.size());
                for (size_t i = 0; i < hdros.size(); i++)
                {
                    hdros[i] = hdri;
                    hdros[i].set_compression(gta::none);
                    hdros[i].set_dimensions(dim_sizes.size(), dim_sizes.size() > 0 ? &(dim_sizes[0]) : NULL);
                    uintmax_t hdro_dim = 0;
                    for (uintmax_t j = 0; j < hdri.dimensions(); j++)
                    {
                        if (j != dim)
                        {
                            hdros[i].dimension_taglist(hdro_dim) = hdri.dimension_taglist(j);
                            hdro_dim++;
                        }
                    }
                    tmpfiles[i] = cio::tempfile(PACKAGE_NAME);
                }
                // Write the GTA data to temporary files
                blob element(checked_cast<size_t>(hdri.element_size()));
                std::vector<uintmax_t> indices(hdri.dimensions());
                gta::io_state si;
                std::vector<gta::io_state> sos(hdros.size());
                for (uintmax_t e = 0; e < hdri.elements(); e++)
                {
                    hdri.read_elements(si, fi, 1, element.ptr());
                    linear_index_to_indices(hdri, e, &(indices[0]));
                    size_t i = indices[dim];
                    hdros[i].write_elements(sos[i], tmpfiles[i], 1, element.ptr());
                }
                // Combine the GTA data to a single output stream
                for (size_t i = 0; i < hdros.size(); i++)
                {
                    hdros[i].write_to(stdout);
                    cio::rewind(tmpfiles[i]);
                    hdros[i].copy_data(tmpfiles[i], hdros[i], stdout);
                    cio::close(tmpfiles[i]);
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
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
