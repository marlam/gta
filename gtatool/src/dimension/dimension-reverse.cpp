/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010, 2011
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

    try
    {
        array_loop_t array_loop;
        gta::header hdri, hdro;
        std::string namei, nameo;
        array_loop.start(arguments, "");
        while (array_loop.read(hdri, namei))
        {
            if (!cio::seekable(array_loop.file_in()))
            {
                throw exc(array_loop.filename_in() + ": input is not seekable");
            }
            if (hdri.compression() != gta::none)
            {
                throw exc(namei + ": array is compressed");
            }
            if (!indices.value().empty())
            {
                for (size_t i = 0; i < indices.value().size(); i++)
                {
                    if (indices.value()[i] >= hdri.dimensions())
                    {
                        throw exc(namei + ": array has no dimension " + str::from(indices.value()[i]));
                    }
                }
            }
            uintmax_t data_offset = cio::tell(array_loop.file_in(), array_loop.filename_in());
            hdro = hdri;
            hdro.set_compression(gta::none);
            array_loop.write(hdro, nameo);
            element_loop_t element_loop;
            array_loop.start_element_loop(element_loop, hdri, hdro);
            blob element(checked_cast<size_t>(hdri.element_size()));
            std::vector<uintmax_t> ind(hdro.dimensions());
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
                hdri.read_block(array_loop.file_in(), data_offset, &(ind[0]), &(ind[0]), element.ptr());
                element_loop.write(element.ptr());
            }
            cio::seek(array_loop.file_in(), data_offset, SEEK_SET, array_loop.filename_in());
            array_loop.skip_data(hdri);
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
