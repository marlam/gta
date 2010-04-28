/*
 * create.cpp
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
#include <limits>

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "opt.h"
#include "cio.h"
#include "str.h"
#include "checked_cast.h"

#include "lib.h"


extern "C" void gtatool_create_help(void)
{
    msg::req_txt(
            "create -d|--dimensions=<d0>[,<d1>[,...]] -c|--components=<c0>[,<c1>[,...]] "
            "[-v|--value=<v0>[,<v1>[,...]]] [-n|--n=<n>] [<output-file>]\n"
            "\n"
            "Creates n GTAs and writes them to standard output or the given output file.\n"
            "Default is n=1. "
            "The dimensions and components must be given as comma-separated lists. "
            "An initial value for all array elements can be given as a comma-separated list, "
            "with one entry for each element component. "
            "The default initial value is zero for all element components.\n"
            "Example: -d 256,128 -c uint8,uint8,uint8 -v 32,64,128");
}

extern "C" int gtatool_create(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::tuple<uintmax_t> dimensions("dimensions", 'd', opt::required, 1, std::numeric_limits<uintmax_t>::max());
    options.push_back(&dimensions);
    opt::string components("components", 'c', opt::required);
    options.push_back(&components);
    opt::string value("value", 'v', opt::optional);
    options.push_back(&value);
    opt::val<uintmax_t> n("n", 'n', opt::optional, 1);
    options.push_back(&n);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 0, 1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_create_help();
        return 0;
    }

    FILE *fo = stdout;
    std::string ofilename("standard output");
    try
    {
        if (arguments.size() == 1)
        {
            ofilename = arguments[0];
            fo = cio::open(ofilename, "w");
        }
        if (cio::isatty(fo))
        {
            throw exc("refusing to write to a tty");
        }
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        return 1;
    }

    try
    {
        gta::header hdr;
        hdr.set_dimensions(dimensions.value().size(), &(dimensions.value()[0]));
        std::vector<gta::type> comp_types;
        std::vector<uintmax_t> comp_sizes;
        typelist_from_string(components.value(), &comp_types, &comp_sizes);
        hdr.set_components(comp_types.size(), &(comp_types[0]), comp_sizes.size() == 0 ? NULL : &(comp_sizes[0]));
        blob v(checked_cast<size_t>(hdr.element_size()));
        if (value.value().empty())
        {
            memset(v.ptr(), 0, hdr.element_size());
        }
        else
        {
            valuelist_from_string(value.value(), comp_types, comp_sizes, v.ptr());
        }
        for (uintmax_t i = 0; i < n.value(); i++)
        {
            hdr.write_to(fo);
            gta::io_state so;
            for (uintmax_t j = 0; j < hdr.elements(); j++)
            {
                hdr.write_elements(so, fo, 1, v.ptr());
            }
        }
        if (fo != stdout)
        {
            cio::close(fo);
        }
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        return 1;
    }

    return 0;
}
