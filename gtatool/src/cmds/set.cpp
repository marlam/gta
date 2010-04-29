/*
 * set.cpp
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
#include "blob.h"
#include "opt.h"
#include "cio.h"
#include "str.h"
#include "intcheck.h"

#include "lib.h"


extern "C" void gtatool_set_help(void)
{
    msg::req_txt(
            "set [-i|--index=<i0>[,<i1>[,...]]] -s|--source=<file> [<files>...]\n"
            "\n"
            "Replaces a subset of the input arrays with the given source array. "
            "The source array will be placed at the given index, or at the origin if no index is given. "
            "Parts of the source array that do not fit into the input array(s) are ignored.\n"
            "Example: set -i 20,20 -s img40x40.gta img100x100.gta > img.gta");
}

extern "C" int gtatool_set(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::tuple<uintmax_t> index("index", 'i', opt::optional);
    options.push_back(&index);
    opt::string source("source", 's', opt::required);
    options.push_back(&source);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_set_help();
        return 0;
    }

    if (cio::isatty(stdout))
    {
        msg::err("refusing to write to a tty");
        return 1;
    }

    try
    {
        gta::header hdrs;
        FILE *fs = cio::open(source.value(), "r");
        if (!cio::seekable(fs))
        {
            throw exc(source.value() + " is not seekable");
        }
        hdrs.read_from(fs);
        if (hdrs.data_is_chunked())
        {
            throw exc(source.value() + ": GTA is compressed");
        }
        if (!index.value().empty())
        {
            if (index.value().size() != hdrs.dimensions())
            {
                throw exc("index and source GTA have incompatible number of dimensions");
            }
        }
        uintmax_t source_data_offset = cio::tell(fs, source.value());

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
                // Determine compatibility
                if (hdri.dimensions() != hdrs.dimensions())
                {
                    throw exc(finame + ": GTA has incompatible number of dimensions");
                }
                if (hdri.components() != hdrs.components())
                {
                    throw exc(finame + ": GTA has incompatible element components");
                }
                for (uintmax_t i = 0; i < hdri.components(); i++)
                {
                    if (hdri.component_type(i) != hdrs.component_type(i)
                            || hdri.component_size(i) != hdrs.component_size(i))
                    {
                        throw exc(finame + ": GTA has incompatible element components");
                    }
                }
                // Write the GTA header
                hdro = hdri;
                hdro.set_compression(gta::none);
                hdro.write_to(stdout);
                // Manipulate the GTA data
                blob element(checked_cast<size_t>(hdri.element_size()));
                std::vector<uintmax_t> in_index(hdri.dimensions());
                std::vector<uintmax_t> start_index(hdri.dimensions());
                for (size_t i = 0; i < start_index.size(); i++)
                {
                    start_index[i] = (index.value().empty() ? 0 : index.value()[i]);
                }
                std::vector<uintmax_t> source_index(hdrs.dimensions());
                gta::io_state si, so;
                for (uintmax_t e = 0; e < hdro.elements(); e++)
                {
                    hdri.read_elements(si, fi, 1, element.ptr());
                    linear_index_to_indices(hdri, e, &(in_index[0]));
                    bool replace = true;
                    for (size_t i = 0; i < in_index.size(); i++)
                    {
                        if (in_index[i] < start_index[i] || in_index[i] >= start_index[i] + hdrs.dimension_size(i))
                        {
                            replace = false;
                            break;
                        }
                        else
                        {
                            source_index[i] = in_index[i] - start_index[i];
                        }
                    }
                    if (replace)
                    {
                        hdrs.read_block(fs, source_data_offset, &(source_index[0]), &(source_index[0]), element.ptr());
                    }
                    hdro.write_elements(so, stdout, 1, element.ptr());
                }
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
