/*
 * info.cpp
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
#include "opt.h"
#include "cio.h"
#include "str.h"

#include "lib.h"


extern "C" void gtatool_info_help(void)
{
    msg::req_txt(
            "info [<files...>]\n"
            "\n"
            "Print information about GTAs.");
}

extern "C" int gtatool_info(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_info_help();
        return 0;
    }

    try
    {
        gta::header hdr;
        // Loop over all input files
        size_t arg = 0;
        do
        {
            std::string finame = (arguments.size() == 0 ? "standard input" : arguments[arg]);
            FILE *fi = (arguments.size() == 0 ? stdin : cio::open(finame, "r"));

            // Loop over all GTAs inside the current file
            uintmax_t array = 0;
            while (cio::has_more(fi, finame))
            {
                // Read the GTA header and print information
                hdr.read_from(fi);
                std::stringstream dimensions;
                for (uintmax_t i = 0; i < hdr.dimensions(); i++)
                {
                    dimensions << hdr.dimension_size(i);
                    if (i < hdr.dimensions() - 1)
                    {
                        dimensions << "x";
                    }
                }
                std::stringstream components;
                for (uintmax_t i = 0; i < hdr.components(); i++)
                {
                    components << type_to_string(hdr.component_type(i), hdr.component_size(i));
                    if (i < hdr.components() - 1)
                    {
                        components << ",";
                    }
                }
                if (hdr.data_size() == 0)
                {
                    msg::req("%s array %llu:", finame.c_str(),
                            static_cast<unsigned long long>(array));
                    msg::req("    empty array");
                }
                else
                {
                    msg::req("%s array %llu: %llu bytes (%s)", finame.c_str(),
                            static_cast<unsigned long long>(array),
                            static_cast<unsigned long long>(hdr.data_size()),
                            str::human_readable_memsize(hdr.data_size()).c_str());
                    msg::req("    %s elements of type %s",
                            dimensions.str().c_str(),
                            components.str().c_str());
                }
                for (uintmax_t i = 0; i < hdr.global_taglist().tags(); i++)
                {
                    msg::req("        %s=%s",
                            str::sanitize(hdr.global_taglist().name(i)).c_str(),
                            str::sanitize(hdr.global_taglist().value(i)).c_str());
                }
                for (uintmax_t i = 0; i < hdr.dimensions(); i++)
                {
                    msg::req("    dimension %llu: %llu",
                            static_cast<unsigned long long>(i),
                            static_cast<unsigned long long>(hdr.dimension_size(i)));
                    for (uintmax_t j = 0; j < hdr.dimension_taglist(i).tags(); j++)
                    {
                        msg::req("        %s=%s",
                                str::sanitize(hdr.dimension_taglist(i).name(j)).c_str(),
                                str::sanitize(hdr.dimension_taglist(i).value(j)).c_str());
                    }
                }
                for (uintmax_t i = 0; i < hdr.components(); i++)
                {
                    msg::req("    element component %llu: %s, %s",
                            static_cast<unsigned long long>(i),
                            type_to_string(hdr.component_type(i), hdr.component_size(i)).c_str(),
                            str::human_readable_memsize(hdr.component_size(i)).c_str());
                    for (uintmax_t j = 0; j < hdr.component_taglist(i).tags(); j++)
                    {
                        msg::req("        %s=%s",
                                str::sanitize(hdr.component_taglist(i).name(j)).c_str(),
                                str::sanitize(hdr.component_taglist(i).value(j)).c_str());
                    }
                }
                // Skip the GTA data
                hdr.skip_data(fi);
                array++;
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
