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
#include "base/opt.h"
#include "base/str.h"

#include "lib.h"


extern "C" void gtatool_compress_help(void)
{
    msg::req_txt(
            "compress [-m|--method=zlib[1-9]|bzip2|xz] [<files>...]\n"
            "\n"
            "Compresses GTAs, with method zlib, bzip2, or xz. The default method is bzip2.\n"
            "The zlib method can optionally be followed by the compression level (1-9). If no level is specified, "
            "the default level is used.");
}

extern "C" int gtatool_compress(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> methods;
    methods.push_back("zlib");
    methods.push_back("zlib1");
    methods.push_back("zlib2");
    methods.push_back("zlib3");
    methods.push_back("zlib4");
    methods.push_back("zlib5");
    methods.push_back("zlib6");
    methods.push_back("zlib7");
    methods.push_back("zlib8");
    methods.push_back("zlib9");
    methods.push_back("bzip2");
    methods.push_back("xz");
    opt::val<std::string> method("method", 'm', opt::optional, methods, "bzip2");
    options.push_back(&method);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_compress_help();
        return 0;
    }
    gta::compression compression =
        (  method.value().compare("zlib") == 0 ? gta::zlib
         : method.value().compare("zlib1") == 0 ? gta::zlib1
         : method.value().compare("zlib2") == 0 ? gta::zlib2
         : method.value().compare("zlib3") == 0 ? gta::zlib3
         : method.value().compare("zlib4") == 0 ? gta::zlib4
         : method.value().compare("zlib5") == 0 ? gta::zlib5
         : method.value().compare("zlib6") == 0 ? gta::zlib6
         : method.value().compare("zlib7") == 0 ? gta::zlib7
         : method.value().compare("zlib8") == 0 ? gta::zlib8
         : method.value().compare("zlib9") == 0 ? gta::zlib9
         : method.value().compare("bzip2") == 0 ? gta::bzip2
         : gta::xz);

    try
    {
        array_loop_t array_loop;
        gta::header hdri, hdro;
        std::string namei, nameo;
        array_loop.start(arguments, "");
        while (array_loop.read(hdri, namei))
        {
            hdro = hdri;
            hdro.set_compression(compression);
            array_loop.write(hdro, nameo);
            array_loop.copy_data(hdri, hdro);
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
