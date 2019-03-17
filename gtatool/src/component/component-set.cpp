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


extern "C" void gtatool_component_set_help(void)
{
    msg::req_txt(
            "component-set [-i|--indices=<i0>[,<i1>[,...]]] [-v|--value=<v0>[,<v1>[,...]]] [<files>...]\n"
            "\n"
            "Sets array element components. The components with the given indices are set to the given values. "
            "By default, all components are set. The default value is zero.\n"
            "Example: component-set -i 0,1,2 -v 128,128,128 rgb.gta > grey.gta");
}

extern "C" int gtatool_component_set(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::tuple<uintmax_t> indices("indices", 'i', opt::optional);
    options.push_back(&indices);
    opt::string value("value", 'v', opt::optional);
    options.push_back(&value);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_component_set_help();
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
            // Determine components to set
            gta::header hdrt;
            std::vector<uintmax_t> current_indices;
            if (indices.value().empty())
            {
                for (uintmax_t i = 0; i < hdri.components(); i++)
                {
                    current_indices.push_back(i);
                }
            }
            else
            {
                current_indices = indices.value();
            }
            std::vector<gta::type> comp_types;
            std::vector<uintmax_t> comp_sizes;
            for (size_t i = 0; i < current_indices.size(); i++)
            {
                if (current_indices[i] >= hdri.components())
                {
                    throw exc(namei + ": array has no component " + str::from(current_indices[i]));
                }
                comp_types.push_back(hdri.component_type(current_indices[i]));
                if (hdri.component_type(current_indices[i]) == gta::blob)
                {
                    comp_sizes.push_back(hdri.component_size(current_indices[i]));
                }
            }
            hdrt.set_components(comp_types.size(), comp_types.data(), comp_sizes.size() == 0 ? NULL : &(comp_sizes[0]));
            blob comp_values(checked_cast<size_t>(hdrt.element_size()));
            if (value.value().empty())
            {
                memset(comp_values.ptr(), 0, hdrt.element_size());
            }
            else
            {
                valuelist_from_string(value.value(), comp_types, comp_sizes, comp_values.ptr());
            }

            hdro = hdri;
            hdro.set_compression(gta::none);
            array_loop.write(hdro, nameo);
            if (hdro.data_size() > 0)
            {
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, hdri, hdro);
                blob element(checked_cast<size_t>(hdri.element_size()));
                for (uintmax_t e = 0; e < hdro.elements(); e++)
                {
                    std::memcpy(element.ptr(), element_loop.read(), hdri.element_size());
                    for (size_t i = 0; i < current_indices.size(); i++)
                    {
                        void *component_dst = hdri.component(element.ptr(), current_indices[i]);
                        void *component_src = hdrt.component(comp_values.ptr(), i);
                        memcpy(component_dst, component_src, hdri.component_size(current_indices[i]));
                    }
                    element_loop.write(element.ptr());
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
