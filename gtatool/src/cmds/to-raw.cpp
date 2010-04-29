/*
 * to-raw.cpp
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

#include <string>
#include <limits>

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "cio.h"
#include "opt.h"
#include "debug.h"
#include "intcheck.h"

#include "lib.h"


extern "C" void gtatool_to_raw_help(void)
{
    msg::req_txt("to-raw [-e|--endianness=little|big] [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to raw binary format.");
}

extern "C" int gtatool_to_raw(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> endiannesses;
    endiannesses.push_back("little");
    endiannesses.push_back("big");
    opt::string endianness("endianness", 'e', opt::optional, endiannesses);
    options.push_back(&endianness);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, 2, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_to_raw_help();
        return 0;
    }

    bool host_endianness;
    if (endianness.values().empty())
    {
        host_endianness = true;
    }
    else
    {
#if WORDS_BIGENDIAN
        host_endianness = (endianness.value().compare("big") == 0);
#else
        host_endianness = (endianness.value().compare("little") == 0);
#endif
    }

    FILE *fi = stdin;
    std::string ifilename("standard input");
    std::string ofilename(arguments[0]);
    try
    {
        if (arguments.size() == 2)
        {
            ifilename = arguments[0];
            fi = cio::open(ifilename, "r");
            ofilename = arguments[1];
        }
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        return 1;
    }

    try
    {
        FILE *fo = cio::open(ofilename, "w");
        gta::header hdr;
        hdr.read_from(fi);
        if (hdr.data_is_chunked())
        {
            throw exc("Cannot export " + ifilename, "Currently only uncompressed GTAs can be exported to raw files");
        }
        blob element(checked_cast<size_t>(hdr.element_size()));
        gta::io_state si;
        for (uintmax_t e = 0; e < hdr.elements(); e++)
        {
            hdr.read_elements(si, fi, 1, element.ptr());
            if (!host_endianness)
            {
                swap_element_endianness(hdr, element.ptr());
            }
            cio::write(element.ptr(), hdr.element_size(), 1, fo, ofilename);
        }
        if (fi != stdin)
        {
            cio::close(fi);
        }
        cio::close(fo);
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        return 1;
    }

    return 0;
}
