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
#include <cctype>

#include <gta/gta.hpp>

#include "base/msg.h"
#include "base/blb.h"
#include "base/opt.h"
#include "base/str.h"
#include "base/chk.h"

#include "lib.h"


extern "C" void gtatool_set_help(void)
{
    msg::req_txt(
            "set [-i|--index=<i0>[,<i1>[,...]]] -s|--source=<file> [<files>...]\n"
            "\n"
            "Replaces a subset of the input arrays with the given source array. "
            "The source array will be placed at the given index, or at the origin if no index is given. "
            "Parts of the source array that do not fit into the input array(s) are ignored.\n"
            "Example: set -i 20,20 -s img40x40.gta img100x100.gta > img.gta");
}

extern "C" int gtatool_set(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::tuple<intmax_t> index("index", 'i', opt::optional);
    options.push_back(&index);
    opt::string source("source", 's', opt::required);
    options.push_back(&source);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_set_help();
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
            array_loop_t array_loop_src;
            gta::header hdr_src;
            std::string name_src;
            array_loop_src.start(source.value(), "");
            if (!array_loop_src.read(hdr_src, name_src))
            {
                throw exc(source.value() + " is empty");
            }
            if (!index.value().empty())
            {
                if (index.value().size() != hdr_src.dimensions())
                {
                    throw exc(name_src + ": incompatible with given index");
                }
            }
            if (hdri.dimensions() != hdr_src.dimensions())
            {
                throw exc(namei + ": incompatible number of dimensions");
            }
            if (hdri.components() != hdr_src.components())
            {
                throw exc(namei + ": incompatible element components");
            }
            for (uintmax_t i = 0; i < hdri.components(); i++)
            {
                if (hdri.component_type(i) != hdr_src.component_type(i)
                        || hdri.component_size(i) != hdr_src.component_size(i))
                {
                    throw exc(namei + ": incompatible element components");
                }
            }
            hdro = hdri;
            hdro.set_compression(gta::none);
            array_loop.write(hdro, nameo);

            if (hdro.data_size() > 0)
            {
                element_loop_t element_loop;
                element_loop_t element_loop_src;
                uintmax_t read_src_elements = 0;
                std::vector<intmax_t> src_index(hdr_src.dimensions());
                std::vector<uintmax_t> out_index(hdro.dimensions());
                array_loop.start_element_loop(element_loop, hdri, hdro);
                array_loop_src.start_element_loop(element_loop_src, hdr_src, gta::header());
                for (uintmax_t linear_out_index = 0; linear_out_index < hdro.elements(); linear_out_index++)
                {
                    hdro.linear_index_to_indices(linear_out_index, &(out_index[0]));
                    bool from_src = true;
                    for (uintmax_t i = 0; i < hdr_src.dimensions(); i++)
                    {
                        if (!index.values().empty())
                        {
                            src_index[i] = checked_sub(checked_cast<intmax_t>(out_index[i]), index.value()[i]);
                        }
                        else
                        {
                            src_index[i] = out_index[i];
                        }
                        if (src_index[i] < 0 || static_cast<uintmax_t>(src_index[i]) >= hdr_src.dimension_size(i))
                        {
                            from_src = false;
                        }
                    }
                    const void *src = element_loop.read();
                    if (from_src)
                    {
                        std::vector<uintmax_t> requested_src_index(src_index.size());
                        for (size_t i = 0; i < requested_src_index.size(); i++)
                        {
                            requested_src_index[i] = src_index[i];
                        }
                        uintmax_t requested_linear_src_index = hdr_src.indices_to_linear_index(&(requested_src_index[0]));
                        // elements are guaranteed to be in ascending order
                        for (uintmax_t i = read_src_elements; i <= requested_linear_src_index; i++)
                        {
                            src = element_loop_src.read();
                        }
                        read_src_elements = requested_linear_src_index + 1;
                    }
                    element_loop.write(src);
                }
                for (uintmax_t i = read_src_elements; i < hdr_src.elements(); i++)
                {
                    element_loop_src.read();
                }
            }
            array_loop_src.finish();
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
