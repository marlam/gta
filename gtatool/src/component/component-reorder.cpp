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

#include "msg.h"
#include "blob.h"
#include "opt.h"
#include "fio.h"
#include "str.h"
#include "intcheck.h"

#include "lib.h"


extern "C" void gtatool_component_reorder_help(void)
{
    msg::req_txt(
            "component-reorder [-i|--indices=<i0>[,<i1>[,...]]] [<files>...]\n"
            "\n"
            "Reorders array element components. The new order is given by the list of component indices. "
            "By default, no change is made.\n"
            "Example: component-reorder -i 2,1,0 rgb.gta > bgr.gta");
}

extern "C" int gtatool_component_reorder(int argc, char *argv[])
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
        gtatool_component_reorder_help();
        return 0;
    }
    for (size_t i = 0; i < indices.value().size(); i++)
    {
        if (indices.value()[i] >= indices.value().size())
        {
            msg::err_txt("invalid index %s in list of %s indices",
                    str::from(indices.value()[i]).c_str(),
                    str::from(indices.value().size()).c_str());
            return 1;
        }
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
            if (!indices.value().empty() && hdri.components() != indices.value().size())
            {
                throw exc(namei + ": array has " + str::from(hdri.components())
                        + " components while list of indices has " + str::from(indices.value().size()));
            }
            // Determine the new component order
            hdro = hdri;
            hdro.set_compression(gta::none);
            if (!indices.value().empty())
            {
                std::vector<gta::type> comp_types;
                std::vector<uintmax_t> comp_sizes;
                for (size_t i = 0; i < indices.value().size(); i++)
                {
                    comp_types.push_back(hdri.component_type(indices.value()[i]));
                    if (hdri.component_type(indices.value()[i]) == gta::blob)
                    {
                        comp_sizes.push_back(hdri.component_size(indices.value()[i]));
                    }
                }
                hdro.set_components(comp_types.size(), &(comp_types[0]), comp_sizes.size() == 0 ? NULL : &(comp_sizes[0]));
                for (size_t i = 0; i < indices.value().size(); i++)
                {
                    hdro.component_taglist(i) = hdri.component_taglist(indices.value()[i]);
                }
            }

            array_loop.write(hdro, nameo);
            if (hdro.data_size() > 0)
            {
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, hdri, hdro);
                blob element_out(checked_cast<size_t>(hdro.element_size()));
                for (uintmax_t e = 0; e < hdro.elements(); e++)
                {
                    const void *element_in = element_loop.read();
                    if (!indices.value().empty())
                    {
                        for (uintmax_t i = 0; i < hdro.components(); i++)
                        {
                            std::memcpy(hdro.component(element_out.ptr(), i),
                                    hdri.component(element_in, indices.value()[i]),
                                hdro.component_size(i));
                        }
                    }
                    else
                    {
                        std::memcpy(element_out.ptr(), element_in, hdro.element_size());
                    }
                    element_loop.write(element_out.ptr());
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
