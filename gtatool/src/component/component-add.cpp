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
#include "fio.h"
#include "str.h"
#include "intcheck.h"

#include "lib.h"


extern "C" void gtatool_component_add_help(void)
{
    msg::req_txt(
            "component-add -c|--components=<c0>[,<c1>[,...]] [-i|--index=<i>] "
            "[-v|--value=<v0>[,<v1>[,...]]] [<files>...]\n"
            "\n"
            "Adds array element components. The given components are inserted at the given index. "
            "The default is to append them. The initial value of the components can be specified. The "
            "default is zero.\n"
            "Example: component-add -c uint8 -i 0 -v 255 gb.gta > rgb.gta");
}

extern "C" int gtatool_component_add(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::string components("components", 'c', opt::required);
    options.push_back(&components);
    opt::val<uintmax_t> index("index", 'i', opt::optional);
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
        gtatool_component_add_help();
        return 0;
    }

    try
    {
        gta::header hdrt;
        std::vector<gta::type> comp_types;
        std::vector<uintmax_t> comp_sizes;
        typelist_from_string(components.value(), &comp_types, &comp_sizes);
        hdrt.set_components(comp_types.size(), &(comp_types[0]), comp_sizes.size() == 0 ? NULL : &(comp_sizes[0]));
        blob comp_values(checked_cast<size_t>(hdrt.element_size()));
        if (value.value().empty())
        {
            memset(comp_values.ptr(), 0, hdrt.element_size());
        }
        else
        {
            valuelist_from_string(value.value(), comp_types, comp_sizes, comp_values.ptr());
        }

        array_loop_t array_loop;
        gta::header hdri, hdro;
        std::string namei, nameo;
        array_loop.start(arguments, "");
        while (array_loop.read(hdri, namei))
        {
            hdro = hdri;
            hdro.set_compression(gta::none);
            // Add components
            std::vector<gta::type> hdro_comp_types;
            std::vector<uintmax_t> hdro_comp_sizes;
            uintmax_t hdro_new_comp_index;
            if (index.values().empty())
            {
                hdro_new_comp_index = hdri.components();
            }
            else
            {
                if (index.value() > hdri.components())
                {
                    throw exc(namei + ": array has less than " + str::from(index.value()) + " components");
                }
                hdro_new_comp_index = index.value();
            }
            for (uintmax_t i = 0; i < hdro_new_comp_index; i++)
            {
                hdro_comp_types.push_back(hdri.component_type(i));
                if (hdri.component_type(i) == gta::blob)
                {
                    hdro_comp_sizes.push_back(hdri.component_size(i));
                }
            }
            uintmax_t blob_size_index = 0;
            for (uintmax_t i = hdro_new_comp_index;
                    i < checked_add(hdro_new_comp_index, static_cast<uintmax_t>(comp_types.size())); i++)
            {
                hdro_comp_types.push_back(comp_types[i - hdro_new_comp_index]);
                if (comp_types[i - hdro_new_comp_index] == gta::blob)
                {
                    hdro_comp_sizes.push_back(comp_sizes[blob_size_index++]);
                }
            }
            for (uintmax_t i = hdro_new_comp_index + comp_types.size();
                    i < checked_add(hdri.components(), static_cast<uintmax_t>(comp_types.size())); i++)
            {
                hdro_comp_types.push_back(hdri.component_type(i - comp_types.size()));
                if (hdri.component_type(i - comp_types.size()) == gta::blob)
                {
                    hdro_comp_sizes.push_back(hdri.component_size(i - comp_types.size()));
                }
            }
            hdro.set_components(hdro_comp_types.size(), &(hdro_comp_types[0]),
                    (hdro_comp_sizes.size() > 0 ? &(hdro_comp_sizes[0]) : NULL));
            for (uintmax_t i = 0; i < hdro.components(); i++)
            {
                if (i < hdro_new_comp_index)
                {
                    hdro.component_taglist(i) = hdri.component_taglist(i);
                }
                else if (i >= hdro_new_comp_index + comp_types.size())
                {
                    hdro.component_taglist(i) = hdri.component_taglist(i - comp_types.size());
                }
            }

            array_loop.write(hdro, nameo);
            element_loop_t element_loop;
            array_loop.start_element_loop(element_loop, hdri, hdro);
            blob element_out(checked_cast<size_t>(hdro.element_size()));
            size_t old_comp_pre_size = 0;
            for (uintmax_t i = 0; i < hdro_new_comp_index; i++)
            {
                old_comp_pre_size += hdro.component_size(i);
            }
            for (uintmax_t e = 0; e < hdro.elements(); e++)
            {
                const void *element_in = element_loop.read();
                std::memcpy(element_out.ptr(), element_in, old_comp_pre_size);
                std::memcpy(element_out.ptr(old_comp_pre_size), comp_values.ptr(), hdrt.element_size());
                std::memcpy(element_out.ptr(old_comp_pre_size + hdrt.element_size()),
                        static_cast<const char *>(element_in) + old_comp_pre_size,
                        hdri.element_size() - old_comp_pre_size);
                element_loop.write(element_out.ptr());
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
