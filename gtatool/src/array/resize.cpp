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


extern "C" void gtatool_resize_help(void)
{
    msg::req_txt(
            "resize -d|--dimensions=<d0>[,<d1>[,...]] [-i|--index=<i0>[,<i1>[,...]]] [-v|--value=<v0>[,<v1>[,...]]] [<files>...]\n"
            "\n"
            "Resizes input arrays to the given size. "
            "The original data will be placed at the given index (which may include negative components), or at the origin if no index is given. "
            "Areas of the original array that do not fit in the resulting array are discarded. "
            "Empty areas in the resulting array will be filled with the given value, or zero if no value is given.\n" 
            "Example: resize -d 100,100 -i -50,-50 < img200x200.gta > center100x100.gta");
}

extern "C" int gtatool_resize(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::tuple<uintmax_t> dimensions("dimensions", 'd', opt::required, 1, std::numeric_limits<intmax_t>::max() / 2 - 1);
    options.push_back(&dimensions);
    opt::tuple<intmax_t> index("index", 'i', opt::optional);
    options.push_back(&index);
    opt::string value("value", 'v', opt::optional);
    options.push_back(&value);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_resize_help();
        return 0;
    }
    if (!index.values().empty() && index.value().size() != dimensions.value().size())
    {
        msg::err_txt("the index must have the same dimensionality as the resized array");
        return 1;
    }

    try
    {
        array_loop_t array_loop;
        gta::header hdri, hdro;
        std::string namei, nameo;
        array_loop.start(arguments, "");
        while (array_loop.read(hdri, namei))
        {
            if (hdri.dimensions() != dimensions.value().size())
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
            hdro.set_dimensions(hdri.dimensions(), &(dimensions.value()[0]));
            for (uintmax_t i = 0; i < hdri.dimensions(); i++)
            {
                hdro.dimension_taglist(i) = hdri.dimension_taglist(i);
            }
            array_loop.write(hdro, nameo);

            element_loop_t element_loop;
            uintmax_t read_in_elements = 0;
            std::vector<intmax_t> in_index(hdri.dimensions());
            std::vector<uintmax_t> out_index(hdro.dimensions());
            array_loop.start_element_loop(element_loop, hdri, hdro);
            for (uintmax_t linear_out_index = 0; linear_out_index < hdro.elements(); linear_out_index++)
            {
                hdro.linear_index_to_indices(linear_out_index, &(out_index[0]));
                bool from_input = true;
                for (uintmax_t i = 0; i < hdri.dimensions(); i++)
                {
                    if (!index.values().empty())
                    {
                        in_index[i] = checked_sub(checked_cast<intmax_t>(out_index[i]), index.value()[i]);
                    }
                    else
                    {
                        in_index[i] = out_index[i];
                    }
                    if (in_index[i] < 0 || static_cast<uintmax_t>(in_index[i]) >= hdri.dimension_size(i))
                    {
                        from_input = false;
                    }
                }
                const void *src = NULL;
                if (from_input)
                {
                    std::vector<uintmax_t> requested_in_index(in_index.size());
                    for (size_t i = 0; i < requested_in_index.size(); i++)
                    {
                        requested_in_index[i] = in_index[i];
                    }
                    uintmax_t requested_linear_in_index = hdri.indices_to_linear_index(&(requested_in_index[0]));
                    // elements are guaranteed to be in ascending order
                    for (uintmax_t i = read_in_elements; i <= requested_linear_in_index; i++)
                    {
                        src = element_loop.read();
                    }
                    read_in_elements = requested_linear_in_index + 1;
                }
                else
                {
                    src = v.ptr();
                }
                element_loop.write(src);
            }
            for (uintmax_t i = read_in_elements; i < hdri.elements(); i++)
            {
                element_loop.read();
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
