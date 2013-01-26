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

    try
    {
        array_loop_t array_loop;
        gta::header hdri, hdro;
        std::string namei, nameo;
        array_loop.start(arguments, "");
        while (array_loop.read(hdri, namei))
        {
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
                            throw exc(namei + ": array has no component " + str::from(keep.value()[j]));
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
                            throw exc(namei + ": array has no component " + str::from(drop.value()[j]));
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

            array_loop.write(hdro, nameo);
            if (hdri.data_size() > 0)
            {
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, hdri, hdro);
                blob element_out(checked_cast<size_t>(hdro.element_size()));
                for (uintmax_t e = 0; e < hdro.elements(); e++)
                {
                    const void *element_in = element_loop.read();
                    for (uintmax_t i = 0; i < hdro.components(); i++)
                    {
                        std::memcpy(hdro.component(element_out.ptr(), i),
                                hdri.component(element_in, hdro_comp_indices[i]),
                                hdro.component_size(i));
                    }
                    if (hdro.data_size() > 0)
                    {
                        element_loop.write(element_out.ptr());
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
