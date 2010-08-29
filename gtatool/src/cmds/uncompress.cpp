/*
 * uncompress.cpp
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


extern "C" void gtatool_uncompress_help(void)
{
    msg::req_txt(
            "uncompress [<files...>]\n"
            "\n"
            "Uncompresses GTAs.");
}

extern "C" int gtatool_uncompress(int argc, char *argv[])
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
        gtatool_uncompress_help();
        return 0;
    }

    try
    {
        array_loop_t array_loop(arguments, "");
        gta::header hdri, hdro;
        std::string namei, nameo;
        while (array_loop.read(hdri, namei))
        {
            hdro = hdri;
            hdro.set_compression(gta::none);
            array_loop.write(hdro, nameo);
            array_loop.copy_data(hdri, hdro);
        }
        array_loop.finish();
    }
    catch (exc &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
