/*
 * dimension-reverse.cpp
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


extern "C" void gtatool_dimension_reverse_help(void)
{
    msg::req_txt(
            "dimension-reverse [-i|--indices=<i0>[,<i1>[,...]]] [<files>...]\n"
            "\n"
            "Reverses the given dimensions of the input GTAs.\n"
            "The input GTAs must be uncompressed and seekable for this purpose.\n"
            "Example: dimension-reverse -i 0 image.gta > flipped-image.gta");
}

extern "C" int gtatool_dimension_reverse(int argc, char *argv[])
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
        gtatool_dimension_reverse_help();
        return 0;
    }
    for (size_t i = 0; i < indices.value().size(); i++)
    {
        for (size_t j = 0; j < i; j++)
        {
            if (indices.value()[i] == indices.value()[j])
            {
                msg::err_txt("index %s was used more than once", 
                        str::from(indices.value()[i]).c_str());
                return 1;
            }
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
            if (!cio::seekable(fi))
            {
                throw exc(finame + ": input is not seekable");
            }

            // Loop over all GTAs inside the current file
            uintmax_t array_index = 0;
            while (cio::has_more(fi, finame))
            {
                // Determine the name of the array for error messages
                std::string array_name = finame + " array " + str::from(array_index);
                // Read the GTA header
                hdri.read_from(fi);
                if (hdri.compression() != gta::none)
                {
                    throw exc(array_name + ": array is compressed");
                }
                if (!indices.value().empty())
                {
                    for (size_t i = 0; i < indices.value().size(); i++)
                    {
                        if (indices.value()[i] >= hdri.dimensions())
                        {
                            throw exc(array_name + ": array has no dimension " + str::from(indices.value()[i]));
                        }
                    }
                }
                uintmax_t data_offset = cio::tell(fi, finame);
                // Write the GTA header
                hdro = hdri;
                hdro.set_compression(gta::none);
                hdro.write_to(gtatool_stdout);
                // Manipulate the GTA data
                blob element(checked_cast<size_t>(hdri.element_size()));
                std::vector<uintmax_t> ind(hdro.dimensions());
                gta::io_state so;
                for (uintmax_t e = 0; e < hdro.elements(); e++)
                {
                    hdro.linear_index_to_indices(e, &(ind[0]));
                    if (!indices.value().empty())
                    {
                        for (size_t i = 0; i < indices.value().size(); i++)
                        {
                            uintmax_t j = indices.value()[i];
                            ind[j] = hdri.dimension_size(j) - 1 - ind[j];
                        }
                    }
                    hdri.read_block(fi, data_offset, &(ind[0]), &(ind[0]), element.ptr());
                    hdro.write_elements(so, gtatool_stdout, 1, element.ptr());
                }
                cio::seek(fi, data_offset, SEEK_SET, finame);
                hdri.skip_data(fi);
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
