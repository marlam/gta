/*
 * merge.cpp
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


extern "C" void gtatool_merge_help(void)
{
    msg::req_txt(
            "merge [-d|--dimension=<d>] <files>...\n"
            "\n"
            "Merges the given GTAs by combining them into a new GTA with a larger size in dimension d "
            "(default is d = 0).\n"
            "For example, this can be used to place several 2D images next to each other (dimension 0) "
            "or on top of each other (dimension 1).\n"
            "The components and other dimensions of the input GTAs must match. The first GTA "
            "determines the tags of the output GTA.\n"
            "Example: merge -d 1 top.gta bottom.gta > topbottom.gta");
}

extern "C" int gtatool_merge(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::val<uintmax_t> dimension("dimension", 'd', opt::optional, 0);
    options.push_back(&dimension);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_merge_help();
        return 0;
    }

    if (cio::isatty(stdout))
    {
        msg::err_txt("refusing to write to a tty");
        return 1;
    }

    try
    {
        std::vector<FILE *> fi(arguments.size());
        for (size_t i = 0; i < arguments.size(); i++)
        {
            fi[i] = cio::open(arguments[i], "r");
        }
        std::vector<gta::header> hdri(arguments.size());
        uintmax_t array_index = 0;
        while (cio::has_more(fi[0], arguments[0]))
        {
            hdri[0].read_from(fi[0]);
            if (dimension.value() >= hdri[0].dimensions())
            {
                throw exc(arguments[0] + " array " + str::from(array_index) + ": array has no dimension " + str::from(dimension.value()));
            }
            for (size_t i = 1; i < arguments.size(); i++)
            {
                hdri[i].read_from(fi[i]);
                if (hdri[i].components() != hdri[0].components())
                {
                    throw exc(arguments[i] + " array " + str::from(array_index) + ": incompatible array");
                }
                for (uintmax_t c = 0; c < hdri[0].dimensions(); c++)
                {
                    if (hdri[i].component_type(c) != hdri[0].component_type(c)
                            || hdri[i].component_size(c) != hdri[0].component_size(c))
                    {
                        throw exc(arguments[i] + " array " + str::from(array_index) + ": incompatible array");
                    }
                }
                if (hdri[i].dimensions() != hdri[0].dimensions())
                {
                    throw exc(arguments[i] + " array " + str::from(array_index) + ": incompatible array");
                }
                for (uintmax_t d = 0; d < hdri[0].dimensions(); d++)
                {
                    if (d == dimension.value())
                    {
                        continue;
                    }
                    if (hdri[i].dimension_size(d) != hdri[0].dimension_size(d))
                    {
                        throw exc(arguments[i] + " array " + str::from(array_index) + ": incompatible array");
                    }
                }
            }
            gta::header hdro;
            hdro.global_taglist() = hdri[0].global_taglist();
            std::vector<uintmax_t> hdro_dim_sizes;
            for (uintmax_t d = 0; d < hdri[0].dimensions(); d++)
            {
                if (d == dimension.value())
                {
                    uintmax_t dim = 0;
                    for (size_t j = 0; j < arguments.size(); j++)
                    {
                        dim = checked_add(dim, hdri[j].dimension_size(d));
                    }
                    hdro_dim_sizes.push_back(dim);
                }
                else
                {
                    hdro_dim_sizes.push_back(hdri[0].dimension_size(d));
                }
            }
            hdro.set_dimensions(hdro_dim_sizes.size(), &(hdro_dim_sizes[0]));
            for (uintmax_t d = 0; d < hdri[0].dimensions(); d++)
            {
                hdro.dimension_taglist(d) = hdri[0].dimension_taglist(d);
            }
            std::vector<gta::type> hdro_comp_types;
            std::vector<uintmax_t> hdro_comp_sizes;
            for (uintmax_t c = 0; c < hdri[0].components(); c++)
            {
                hdro_comp_types.push_back(hdri[0].component_type(c));
                if (hdri[0].component_type(c) == gta::blob)
                {
                    hdro_comp_sizes.push_back(hdri[0].component_size(c));
                }
            }
            hdro.set_components(hdro_comp_types.size(), &(hdro_comp_types[0]),
                    (hdro_comp_sizes.size() > 0 ? &(hdro_comp_sizes[0]) : NULL));
            for (uintmax_t c = 0; c < hdri[0].components(); c++)
            {
                hdro.component_taglist(c) = hdri[0].component_taglist(c);
            }
            hdro.write_to(stdout);
            blob element_buf(checked_cast<size_t>(hdro.element_size()));
            std::vector<uintmax_t> indices(hdro.dimensions());
            std::vector<gta::io_state> si(arguments.size());
            gta::io_state so;
            for (uintmax_t e = 0; e < hdro.elements(); e++)
            {
                linear_index_to_indices(hdro, e, &(indices[0]));
                uintmax_t dim = 0;
                size_t j;
                for (j = 0; j < arguments.size(); j++)
                {
                    dim += hdri[j].dimension_size(dimension.value());
                    if (indices[dimension.value()] < dim)
                    {
                        break;
                    }
                }
                hdri[j].read_elements(si[j], fi[j], 1, element_buf.ptr());
                hdro.write_elements(so, stdout, 1, element_buf.ptr());
            }
            array_index++;
        }
        for (size_t i = 1; i < arguments.size(); i++)
        {
            if (cio::has_more(fi[i], arguments[i]))
            {
                msg::wrn_txt("ignoring additional array(s) from %s", arguments[i].c_str());
            }
        }
        for (size_t i = 0; i < arguments.size(); i++)
        {
            cio::close(fi[i], arguments[i]);
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
