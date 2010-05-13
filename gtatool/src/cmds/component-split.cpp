/*
 * component-split.cpp
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


extern "C" void gtatool_component_split_help(void)
{
    msg::req_txt(
            "component-split [-d|--drop=<index0>[,<index1>...]] [<files>...]\n"
            "\n"
            "Split each input array into multiple arrays by separating its array element components. "
            "A list of components to drop can be given.\n"
            "If you only want to extract a subset of components, use the component-extract command instead.\n"
            "All output arrays will be written into a single stream; if you want separate files, "
            "pipe this stream through the stream-split command.\n"
            "Example:\n"
            "component-split rgba.gta > seaparte-r-g-b-a-arrays.gta");
}

extern "C" int gtatool_component_split(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::tuple<uintmax_t> drop("drop", 'd', opt::optional);
    options.push_back(&drop);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_component_split_help();
        return 0;
    }

    if (cio::isatty(stdout))
    {
        msg::err("refusing to write to a tty");
        return 1;
    }

    try
    {
        gta::header hdri;
        // Loop over all input files
        size_t arg = 0;
        do
        {
            std::string finame = (arguments.size() == 0 ? "standard input" : arguments[arg]);
            FILE *fi = (arguments.size() == 0 ? stdin : cio::open(finame, "r"));

            // Loop over all GTAs inside the current file
            uintmax_t array_index = 0;
            while (cio::has_more(fi, finame))
            {
                // Determine the name of the array for error messages
                std::string array_name = finame + " array " + str::str(array_index);
                // Read the GTA header
                hdri.read_from(fi);
                // Determine output GTAs
                std::vector<uintmax_t> comp_indices;
                for (uintmax_t i = 0; i < hdri.components(); i++)
                {
                    if (!drop.value().empty())
                    {
                        bool drop_this = false;
                        for (size_t j = 0; j < drop.value().size(); j++)
                        {
                            if (drop.value()[j] >= hdri.components())
                            {
                                throw exc(array_name + ": array has no component " + str::str(drop.value()[j]));
                            }
                            if (drop.value()[j] == i)
                            {
                                drop_this = true;
                            }
                        }
                        if (drop_this)
                        {
                            continue;
                        }
                    }
                    comp_indices.push_back(i);
                }
                // Define the GTA headers and create temporary files
                std::vector<gta::header> hdros(comp_indices.size());
                std::vector<FILE *> tmpfiles(comp_indices.size());
                for (size_t i = 0; i < hdros.size(); i++)
                {
                    hdros[i] = hdri;
                    hdros[i].set_compression(gta::none);
                    hdros[i].set_components(hdri.component_type(comp_indices[i]), hdri.component_size(comp_indices[i]));
                    hdros[i].component_taglist(0) = hdri.component_taglist(comp_indices[i]);
                    tmpfiles[i] = cio::tempfile(PACKAGE_NAME);
                }
                // Write the GTA data to temporary files
                blob element_in(checked_cast<size_t>(hdri.element_size()));
                gta::io_state si;
                std::vector<gta::io_state> sos(hdros.size());
                for (uintmax_t e = 0; e < hdri.elements(); e++)
                {
                    hdri.read_elements(si, fi, 1, element_in.ptr());
                    if (hdros.size() > 0)
                    {
                        size_t out_index = 0;
                        size_t out_comp_offset = 0;
                        for (uintmax_t i = 0; i < hdri.components(); i++)
                        {
                            if (i == comp_indices[out_index])
                            {
                                hdros[out_index].write_elements(sos[out_index], tmpfiles[out_index],
                                        1, element_in.ptr(out_comp_offset));
                                out_index++;
                            }
                            out_comp_offset += hdri.component_size(i);
                        }
                    }
                }
                // Combine the GTA data to a single output stream
                for (size_t i = 0; i < hdros.size(); i++)
                {
                    hdros[i].write_to(stdout);
                    cio::rewind(tmpfiles[i]);
                    hdros[i].copy_data(tmpfiles[i], hdros[i], stdout);
                    cio::close(tmpfiles[i]);
                }
                array_index++;
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
