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
        array_loop_t array_loop;
        gta::header hdr;
        std::string name;
        array_loop.start(arguments, "");
        while (array_loop.read(hdr, name))
        {
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
                msg::req(name + ":");
            }
            else
            {
                msg::req(name + ": "
                        + str::from(hdr.data_size()) + " bytes ("
                        + str::human_readable_memsize(hdr.data_size()) + ")");
            }
            msg::req(std::string("    compression: ") +
                    (hdr.compression() == gta::none ? "none"
                     : hdr.compression() == gta::zlib ? "zlib default level"
                     : hdr.compression() == gta::bzip2 ? "bzip2"
                     : hdr.compression() == gta::xz ? "xz"
                     : hdr.compression() == gta::zlib1 ? "zlib level 1"
                     : hdr.compression() == gta::zlib2 ? "zlib level 2"
                     : hdr.compression() == gta::zlib3 ? "zlib level 3"
                     : hdr.compression() == gta::zlib4 ? "zlib level 4"
                     : hdr.compression() == gta::zlib5 ? "zlib level 5"
                     : hdr.compression() == gta::zlib6 ? "zlib level 6"
                     : hdr.compression() == gta::zlib7 ? "zlib level 7"
                     : hdr.compression() == gta::zlib8 ? "zlib level 8"
                     : hdr.compression() == gta::zlib9 ? "zlib level 9" : "unknown"));
            if (hdr.data_size() == 0)
            {
                msg::req("    empty array");
            }
            else
            {
                msg::req(std::string("    ") + dimensions.str() + " elements of type " + components.str());
            }
            for (uintmax_t i = 0; i < hdr.global_taglist().tags(); i++)
            {
                msg::req(std::string("        ")
                        + from_utf8(hdr.global_taglist().name(i)) + "=" + from_utf8(hdr.global_taglist().value(i)));
            }
            for (uintmax_t i = 0; i < hdr.dimensions(); i++)
            {
                msg::req(std::string("    dimension ") + str::from(i) + ": " + str::from(hdr.dimension_size(i)));
                for (uintmax_t j = 0; j < hdr.dimension_taglist(i).tags(); j++)
                {
                    msg::req(std::string("        ")
                            + from_utf8(hdr.dimension_taglist(i).name(j)) + "=" + from_utf8(hdr.dimension_taglist(i).value(j)));
                }
            }
            for (uintmax_t i = 0; i < hdr.components(); i++)
            {
                msg::req(std::string("    element component ") + str::from(i) + ": "
                        + type_to_string(hdr.component_type(i), hdr.component_size(i)) + ", "
                        + str::human_readable_memsize(hdr.component_size(i)));
                for (uintmax_t j = 0; j < hdr.component_taglist(i).tags(); j++)
                {
                    msg::req(std::string("        ")
                            + from_utf8(hdr.component_taglist(i).name(j)) + "=" + from_utf8(hdr.component_taglist(i).value(j)));
                }
            }
            array_loop.skip_data(hdr);
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
