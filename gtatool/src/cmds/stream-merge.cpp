/*
 * stream-merge.cpp
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


extern "C" void gtatool_stream_merge_help(void)
{
    msg::req_txt(
            "stream-merge [<files...>]\n"
            "\n"
            "Combines all input arrays into one stream that is written to standard output.\n"
            "The arrays are uncompressed during merging (which means that this command "
            "is currently equivalent to the uncompress command).\n"
            "If you want to merge GTAs without modifying them, you can simple use 'cat'.");
}

extern "C" int gtatool_stream_merge(int argc, char *argv[])
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
        gtatool_stream_merge_help();
        return 0;
    }

    if (cio::isatty(gtatool_stdout))
    {
        msg::err_txt("refusing to write to a tty");
        return 1;
    }

    try
    {
        gta::header hdri, hdro;
        // Loop over all input files
        size_t arg = 0;
        do
        {
            std::string finame = (arguments.size() == 0 ? "standard input" : arguments[arg]);
            FILE *fi = (arguments.size() == 0 ? gtatool_stdin : cio::open(finame, "r"));

            // Loop over all GTAs inside the current file
            while (cio::has_more(fi, finame))
            {
                // Read the GTA header
                hdri.read_from(fi);
                // Set compression
                hdro = hdri;
                hdro.set_compression(gta::none);
                // Write the GTA header
                hdro.write_to(gtatool_stdout);
                // Copy the GTA data
                hdri.copy_data(fi, hdro, gtatool_stdout);
            }
            if (fi != gtatool_stdin)
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
