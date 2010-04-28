/*
 * dimension-reorder.cpp
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
#include "checked_cast.h"

#include "lib.h"

#ifndef EOVERFLOW
#   define EOVERFLOW EFBIG
#endif


extern "C" void gtatool_dimension_reorder_help(void)
{
    msg::req_txt(
            "dimension-reorder [-i|--indices=<i0>[,<i1>[,...]]] [<files>...]\n"
            "\n"
            "Reorders the dimensions of the input GTAs into the given new order.\n"
            "The input GTAs must be uncompressed and seekable for this purpose.\n"
            "Example: dimension-reorder -i 1,0 matrix.gta > transposed.gta");
}

extern "C" int gtatool_dimension_reorder(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::tuple<uintmax_t> indices("indices", 'i', opt::optional);
    options.push_back(&indices);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_dimension_reorder_help();
        return 0;
    }
    for (size_t i = 0; i < indices.value().size(); i++)
    {
        if (indices.value()[i] >= indices.value().size())
        {
            msg::err("invalid index %s in list of %s indices",
                    str::str(indices.value()[i]).c_str(),
                    str::str(indices.value().size()).c_str());
            return 1;
        }
        for (size_t j = 0; j < i; j++)
        {
            if (indices.value()[i] == indices.value()[j])
            {
                msg::err("index %s was used more than once", 
                        str::str(indices.value()[i]).c_str());
                return 1;
            }
        }
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
            if (!cio::seekable(fi))
            {
                throw exc(finame + ": input is not seekable");
            }

            // Loop over all GTAs inside the current file
            while (cio::has_more(fi, finame))
            {
                // Read the GTA header
                hdri.read_from(fi);
                if (hdri.data_is_chunked())
                {
                    throw exc(finame + ": GTA is compressed");
                }
                if (!indices.value().empty() && hdri.dimensions() != indices.value().size())
                {
                    throw exc(finame + ": GTA has " + str::str(hdri.dimensions()) 
                            + " dimensions while list of indices has " + str::str(indices.value().size()));
                }
                uintmax_t data_offset = cio::tell(fi, finame);
                // Reorder the dimensions
                hdro = hdri;
                hdro.set_compression(gta::none);
                if (!indices.value().empty())
                {
                    std::vector<uintmax_t> dim_sizes;
                    for (size_t i = 0; i < indices.value().size(); i++)
                    {
                        dim_sizes.push_back(hdri.dimension_size(indices.value()[i]));
                    }
                    hdro.set_dimensions(dim_sizes.size(), &(dim_sizes[0]));
                    for (size_t i = 0; i < indices.value().size(); i++)
                    {
                        hdro.dimension_taglist(i) = hdri.dimension_taglist(indices.value()[i]);
                    }
                }
                // Write the GTA header
                hdro.write_to(stdout);
                // Manipulate the GTA data
                blob element(checked_cast<size_t>(hdri.element_size()));
                std::vector<uintmax_t> in_indices(hdri.dimensions());
                std::vector<uintmax_t> out_indices(hdro.dimensions());
                gta::io_state so;
                for (uintmax_t e = 0; e < hdro.elements(); e++)
                {
                    linear_index_to_indices(hdro, e, &(out_indices[0]));
                    for (size_t i = 0; i < out_indices.size(); i++)
                    {
                        if (!indices.value().empty())
                        {
                            in_indices[indices.value()[i]] = out_indices[i];
                        }
                        else
                        {
                            in_indices[i] = out_indices[i];
                        }
                    }
                    hdri.read_block(fi, data_offset, &(in_indices[0]), &(in_indices[0]), element.ptr());
                    hdro.write_elements(so, stdout, 1, element.ptr());
                }
                cio::seek(fi, data_offset, SEEK_SET, finame);
                hdri.skip_data(fi);
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
