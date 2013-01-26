/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2011, 2013
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
#include "str.h"
#include "intcheck.h"

#include "lib.h"


extern "C" void gtatool_dimension_flatten_help(void)
{
    msg::req_txt(
            "dimension-flatten [-p|--prepend-coordinates] [<files>...]\n"
            "\n"
            "Flattens the input GTAs by copying all input elements into one-dimensional arrays.\n"
            "If -p is given, the original coordinates of an array element are prepended to the element "
            "so that they are not lost. For example, an element (x,y) in an image that stores R,G,B values "
            "will result in an element that stores X,Y,R,G,B values in the one-dimensional output array.");
}

extern "C" int gtatool_dimension_flatten(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::flag prepend_coordinates("preprend-coordinates", 'p', opt::optional);
    options.push_back(&prepend_coordinates);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_dimension_flatten_help();
        return 0;
    }

    try
    {
        array_loop_t array_loop;
        gta::header hdri, hdro;
        std::string namei, nameo;
        blob eo;
        std::vector<uintmax_t> index;

        array_loop.start(arguments, "");
        while (array_loop.read(hdri, namei))
        {
            hdro = hdri;
            hdro.set_compression(gta::none);
            if (hdri.elements() > 0)
            {
                hdro.set_dimensions(hdri.elements());
            }
            if (prepend_coordinates.value())
            {
                std::vector<gta::type> hdro_comp_types;
                std::vector<uintmax_t> hdro_comp_sizes;
                hdro_comp_types.resize(checked_cast<size_t>(hdri.dimensions()), gta::uint64);
                for (uintmax_t c = 0; c < hdri.components(); c++)
                {
                    hdro_comp_types.push_back(hdri.component_type(c));
                    if (hdri.component_type(c) == gta::blob)
                    {
                        hdro_comp_sizes.push_back(hdri.component_size(c));
                    }
                }
                hdro.set_components(hdro_comp_types.size(), &(hdro_comp_types[0]),
                        hdro_comp_sizes.size() == 0 ? NULL : &(hdro_comp_sizes[0]));
                for (uintmax_t c = 0; c < hdri.components(); c++)
                {
                    hdro.component_taglist(hdri.dimensions() + c) = hdri.component_taglist(c);
                }
                eo.resize(checked_cast<size_t>(hdro.element_size()));
                index.resize(hdri.dimensions());
            }
            array_loop.write(hdro, nameo);
            if (hdro.data_size() > 0)
            {
                if (hdri.element_size() > 0)
                {
                    element_loop_t element_loop;
                    array_loop.start_element_loop(element_loop, hdri, hdro);
                    for (uintmax_t e = 0; e < hdro.elements(); e++)
                    {
                        if (prepend_coordinates.value())
                        {
                            hdri.linear_index_to_indices(e, &(index[0]));
                            for (size_t i = 0; i < index.size(); i++)
                            {
                                eo.ptr<uint64_t>()[i] = checked_cast<uint64_t>(index[i]);
                            }
                            std::memcpy(eo.ptr(hdro.element_size() - hdri.element_size()),
                                    element_loop.read(), hdri.element_size());
                            element_loop.write(eo.ptr());
                        }
                        else
                        {
                            element_loop.write(element_loop.read());
                        }
                    }
                }
                else
                {
                    // There were no element components, and now there are only the prepended
                    // coordinates. Generate them.
                    element_loop_t element_loop;
                    array_loop.start_element_loop(element_loop, hdri, hdro);
                    for (uintmax_t e = 0; e < hdro.elements(); e++)
                    {
                        hdri.linear_index_to_indices(e, &(index[0]));
                        for (size_t i = 0; i < index.size(); i++)
                        {
                            eo.ptr<uint64_t>()[i] = checked_cast<uint64_t>(index[i]);
                        }
                        element_loop.write(eo.ptr());
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
