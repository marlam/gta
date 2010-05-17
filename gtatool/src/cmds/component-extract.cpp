/*
 * component-extract.cpp
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


extern "C" void gtatool_component_extract_help(void)
{
    msg::req_txt(
            "component-extract [-k|--keep=<index0>[,<index1>...]] [<files>...]\n"
            "component-extract [-d|--drop=<index0>[,<index1>...]] [<files>...]\n"
            "\n"
            "Extract array element components. Either a list of components to keep "
            "or a list of components to drop must be given. The default is to keep "
            "all components, i.e. to change nothing.\n"
            "Examples:\n"
            "component-extract -k 0,3 rgba.gta > ra.gta\n"
            "component-extract -d 1,2 rgba.gta > ra.gta");
}

extern "C" int gtatool_component_extract(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::tuple<uintmax_t> keep("keep", 'k', opt::optional);
    options.push_back(&keep);
    opt::tuple<uintmax_t> drop("drop", 'd', opt::optional);
    options.push_back(&drop);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_component_extract_help();
        return 0;
    }
    if (!keep.value().empty() && !drop.value().empty())
    {
        msg::err_txt("cannot use both --keep and --drop");
        return 1;
    }

    if (cio::isatty(stdout))
    {
        msg::err_txt("refusing to write to a tty");
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
                // Remove components
                hdro = hdri;
                hdro.set_compression(gta::none);
                std::vector<uintmax_t> hdro_comp_indices;
                std::vector<gta::type> hdro_comp_types;
                std::vector<uintmax_t> hdro_comp_sizes;
                for (uintmax_t i = 0; i < hdri.components(); i++)
                {
                    bool keep_this;
                    if (!keep.value().empty())
                    {
                        keep_this = false;
                        for (size_t j = 0; j < keep.value().size(); j++)
                        {
                            if (keep.value()[j] >= hdri.components())
                            {
                                throw exc(array_name + ": array has no component " + str::from(keep.value()[j]));
                            }
                            if (keep.value()[j] == i)
                            {
                                keep_this = true;
                            }
                        }
                    }
                    else if (!drop.value().empty())
                    {
                        keep_this = true;
                        for (size_t j = 0; j < drop.value().size(); j++)
                        {
                            if (drop.value()[j] >= hdri.components())
                            {
                                throw exc(array_name + ": array has no component " + str::from(drop.value()[j]));
                            }
                            if (drop.value()[j] == i)
                            {
                                keep_this = false;
                            }
                        }
                    }
                    else
                    {
                        keep_this = true;
                    }
                    if (keep_this)
                    {
                        hdro_comp_indices.push_back(i);
                        hdro_comp_types.push_back(hdri.component_type(i));
                        if (hdri.component_type(i) == gta::blob)
                        {
                            hdro_comp_sizes.push_back(hdri.component_size(i));
                        }
                    }
                }
                hdro.set_components(hdro_comp_types.size(), &(hdro_comp_types[0]),
                        (hdro_comp_sizes.size() > 0 ? &(hdro_comp_sizes[0]) : NULL));
                for (size_t i = 0; i < hdro_comp_indices.size(); i++)
                {
                    hdro.component_taglist(i) = hdri.component_taglist(hdro_comp_indices[i]);
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
                    size_t component_in_index = 0;
                    size_t component_out_index = 0;
                    for (size_t i = 0; i < hdro_comp_indices.size(); i++)
                    {
                        for (uintmax_t j = i; j < hdro_comp_indices[i]; j++)
                        {
                            component_in_index += hdri.component_size(i);
                        }
                        memcpy(element_out.ptr(component_out_index),
                                element_in.ptr(component_in_index),
                                hdro.component_size(i));
                        component_in_index += hdro.component_size(i);
                        component_out_index += hdro.component_size(i);
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
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
