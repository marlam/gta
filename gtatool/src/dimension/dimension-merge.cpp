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


extern "C" void gtatool_dimension_merge_help(void)
{
    msg::req_txt(
            "dimension-merge <files>...\n"
            "\n"
            "Merges the given GTAs by combining them into a new GTA with higher dimension. "
            "This can be used to combine several 1D lines to 2D images, or several 2D images "
            "to a 3D volume, and so forth.\n"
            "The dimensions and components of the input GTAs must match. The first GTA "
            "determines the tags of the output GTA.\n"
            "Example: dimension-merge slice0.gta slice1.gta slice2.gta > volume.gta");
}

extern "C" int gtatool_dimension_merge(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_dimension_merge_help();
        return 0;
    }

    try
    {
        std::vector<array_loop_t> array_loops(arguments.size());
        std::vector<gta::header> hdris(arguments.size());
        std::vector<std::string> nameis(arguments.size());
        std::string nameo;
        for (size_t i = 0; i < arguments.size(); i++)
        {
            array_loops[i].start(arguments[i], "");
        }
        while (array_loops[0].read(hdris[0], nameis[0]))
        {
            for (size_t i = 1; i < arguments.size(); i++)
            {
                array_loops[i].read(hdris[i], nameis[i]);
                if (hdris[i].components() != hdris[0].components())
                {
                    throw exc(nameis[i] + ": incompatible array (number of components differs)");
                }
                for (uintmax_t c = 0; c < hdris[0].components(); c++)
                {
                    if (hdris[i].component_type(c) != hdris[0].component_type(c)
                            || hdris[i].component_size(c) != hdris[0].component_size(c))
                    {
                        throw exc(nameis[i] + ": incompatible array (component types differ)");
                    }
                }
                if (hdris[i].dimensions() != hdris[0].dimensions())
                {
                    throw exc(nameis[i] + ": incompatible array (number of dimension differs)");
                }
                for (uintmax_t d = 0; d < hdris[0].dimensions(); d++)
                {
                    if (hdris[i].dimension_size(d) != hdris[0].dimension_size(d))
                    {
                        throw exc(nameis[i] + ": incompatible array (dimension sizes differ)");
                    }
                }
            }
            gta::header hdro;
            hdro.global_taglist() = hdris[0].global_taglist();
            std::vector<uintmax_t> hdro_dim_sizes;
            for (uintmax_t d = 0; d < hdris[0].dimensions(); d++)
            {
                hdro_dim_sizes.push_back(hdris[0].dimension_size(d));
            }
            hdro_dim_sizes.push_back(arguments.size());
            hdro.set_dimensions(hdro_dim_sizes.size(), &(hdro_dim_sizes[0]));
            for (uintmax_t d = 0; d < hdris[0].dimensions(); d++)
            {
                hdro.dimension_taglist(d) = hdris[0].dimension_taglist(d);
            }
            std::vector<gta::type> hdro_comp_types;
            std::vector<uintmax_t> hdro_comp_sizes;
            for (uintmax_t c = 0; c < hdris[0].components(); c++)
            {
                hdro_comp_types.push_back(hdris[0].component_type(c));
                if (hdris[0].component_type(c) == gta::blob)
                {
                    hdro_comp_sizes.push_back(hdris[0].component_size(c));
                }
            }
            hdro.set_components(hdro_comp_types.size(), &(hdro_comp_types[0]),
                    (hdro_comp_sizes.size() > 0 ? &(hdro_comp_sizes[0]) : NULL));
            for (uintmax_t c = 0; c < hdris[0].components(); c++)
            {
                hdro.component_taglist(c) = hdris[0].component_taglist(c);
            }
            array_loops[0].write(hdro, nameo);
            element_loop_t element_loop;
            array_loops[0].start_element_loop(element_loop, hdris[0], hdro);
            for (size_t i = 0; i < arguments.size(); i++)
            {
                element_loop_t element_loop_2;
                array_loops[i].start_element_loop(element_loop_2, hdris[i], hdro);
                for (uintmax_t j = 0; j < hdris[i].elements(); j++)
                {
                    element_loop.write(element_loop_2.read());
                }
            }
        }
        for (size_t i = 0; i < arguments.size(); i++)
        {
            if (i > 0 && array_loops[i].read(hdris[i], nameis[i]))
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
