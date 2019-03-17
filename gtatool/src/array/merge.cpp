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
#include "base/fio.h"
#include "base/str.h"
#include "base/chk.h"

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

    if (fio::isatty(gtatool_stdout))
    {
        msg::err_txt("refusing to write to a tty");
        return 1;
    }

    try
    {
        std::vector<array_loop_t> array_loops(arguments.size());
        std::vector<gta::header> hdri(arguments.size());
        std::vector<std::string> namei(arguments.size());

        for (size_t i = 0; i < arguments.size(); i++)
        {
            array_loops[i].start(arguments[i], "");
        }
        while (array_loops[0].read(hdri[0], namei[0]))
        {
            if (dimension.value() >= hdri[0].dimensions())
            {
                throw exc(namei[0] + ": array has no dimension " + str::from(dimension.value()));
            }
            bool have_input = true;
            for (size_t i = 1; i < arguments.size(); i++)
            {
                if (!array_loops[i].read(hdri[i], namei[i]))
                {
                    have_input = false;
                    break;
                }
                if (hdri[i].components() != hdri[0].components())
                {
                    throw exc(namei[i] + ": incompatible array");
                }
                for (uintmax_t c = 0; c < hdri[0].components(); c++)
                {
                    if (hdri[i].component_type(c) != hdri[0].component_type(c)
                            || hdri[i].component_size(c) != hdri[0].component_size(c))
                    {
                        throw exc(namei[i] + ": incompatible array");
                    }
                }
                if (hdri[i].dimensions() != hdri[0].dimensions())
                {
                    throw exc(namei[i] + ": incompatible array");
                }
                for (uintmax_t d = 0; d < hdri[0].dimensions(); d++)
                {
                    if (d == dimension.value())
                    {
                        continue;
                    }
                    if (hdri[i].dimension_size(d) != hdri[0].dimension_size(d))
                    {
                        throw exc(namei[i] + ": incompatible array");
                    }
                }
            }
            if (!have_input)
            {
                msg::wrn_txt("ignoring additional array(s) from %s", arguments[0].c_str());
                break;
            }
            gta::header hdro;
            std::string nameo;
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
            hdro.set_components(hdro_comp_types.size(), hdro_comp_types.data(),
                    (hdro_comp_sizes.size() > 0 ? &(hdro_comp_sizes[0]) : NULL));
            for (uintmax_t c = 0; c < hdri[0].components(); c++)
            {
                hdro.component_taglist(c) = hdri[0].component_taglist(c);
            }
            array_loops[0].write(hdro, nameo);
            if (hdro.data_size() > 0)
            {
                std::vector<uintmax_t> indices(hdro.dimensions());
                std::vector<element_loop_t> element_loops(arguments.size());
                for (size_t i = 0; i < element_loops.size(); i++)
                {
                    array_loops[i].start_element_loop(element_loops[i], hdri[i], hdro);
                }
                for (uintmax_t e = 0; e < hdro.elements(); e++)
                {
                    hdro.linear_index_to_indices(e, &(indices[0]));
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
                    element_loops[0].write(element_loops[j].read());
                }
            }
        }
        array_loops[0].finish();
        for (size_t i = 1; i < arguments.size(); i++)
        {
            if (array_loops[i].read(hdri[i], namei[i]))
            {
                msg::wrn_txt("ignoring additional array(s) from %s", arguments[i].c_str());
            }
            array_loops[i].finish();
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
