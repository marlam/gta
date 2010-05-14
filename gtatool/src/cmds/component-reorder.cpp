/*
 * component-reorder.cpp
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
            msg::err("invalid index %s in list of %s indices",
                    str::from(indices.value()[i]).c_str(),
                    str::from(indices.value().size()).c_str());
            return 1;
        }
        for (size_t j = 0; j < i; j++)
        {
            if (indices.value()[i] == indices.value()[j])
            {
                msg::err("index %s was used more than once",
                        str::from(indices.value()[i]).c_str());
                return 1;
            }
        }
    }

    if (cio::isatty(stdout))
    {
        msg::err("refusing to write to a tty");
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
            FILE *fi = (arguments.size() == 0 ? stdin : cio::open(finame, "r"));

            // Loop over all GTAs inside the current file
            uintmax_t array_index = 0;
            while (cio::has_more(fi, finame))
            {
                // Determine the name of the array for error messages
                std::string array_name = finame + " array " + str::from(array_index);
                // Read the GTA header
                hdri.read_from(fi);
                if (!indices.value().empty() && hdri.components() != indices.value().size())
                {
                    throw exc(array_name + ": array has " + str::from(hdri.components())
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
                // Write the GTA header
                hdro.write_to(stdout);
                // Manipulate the GTA data
                blob element_in(checked_cast<size_t>(hdri.element_size()));
                blob element_out(checked_cast<size_t>(hdro.element_size()));
                gta::io_state si, so;
                for (uintmax_t e = 0; e < hdro.elements(); e++)
                {
                    hdri.read_elements(si, fi, 1, element_in.ptr());
                    if (!indices.value().empty())
                    {
                        for (uintmax_t i = 0; i < hdro.components(); i++)
                        {
                            memcpy(hdro.component(element_out.ptr(), i),
                                    hdri.component(element_in.ptr(), indices.value()[i]),
                                    hdro.component_size(i));
                        }
                    }
                    else
                    {
                        memcpy(element_out.ptr(), element_in.ptr(), hdro.element_size());
                    }
                    hdro.write_elements(so, stdout, 1, element_out.ptr());
                }
                array_index++;
            }
            if (fi != stdin)
            {
                cio::close(fi);
            }
            arg++;
        }
        while (arg < arguments.size());
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        return 1;
    }

    return 0;
}
