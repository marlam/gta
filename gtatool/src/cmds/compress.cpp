/*
 * compress.cpp
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


extern "C" void gtatool_compress_help(void)
{
    msg::req_txt(
            "compress [--method=zlib|bzip2|xz] [<files>...]\n"
            "\n"
            "Compresses GTAs. The default method is bzip2.");
}

extern "C" int gtatool_compress(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> methods;
    methods.push_back("zlib");
    methods.push_back("bzip2");
    methods.push_back("xz");
    opt::val<std::string> method("method", '\0', opt::optional, methods, "bzip2");
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
        (method.value().compare("zlib") == 0 ? gta::zlib
         : method.value().compare("bzip2") == 0 ? gta::bzip2
         : gta::xz);

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
            while (cio::has_more(fi, finame))
            {
                // Read the GTA header
                hdri.read_from(fi);
                // Set compression
                hdro = hdri;
                hdro.set_compression(compression);
                // Write the GTA header
                hdro.write_to(stdout);
                // Copy the GTA data
                hdri.copy_data(fi, hdro, stdout);
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
