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

#include "msg.h"
#include "blob.h"
#include "opt.h"
#include "fio.h"
#include "str.h"
#include "intcheck.h"

#include "lib.h"


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
            "component-split rgba.gta > separate-r-g-b-a-arrays.gta");
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

    try
    {
        array_loop_t array_loop;
        gta::header hdri, hdro;
        std::string namei, nameo;
        array_loop.start(arguments, "");
        while (array_loop.read(hdri, namei))
        {
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
                            throw exc(namei + ": array has no component " + str::from(drop.value()[j]));
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
            std::vector<std::string> nameos(hdros.size());
            std::vector<FILE *> tmpfiles(hdros.size());
            std::vector<std::string> tmpfilenames(hdros.size());
            std::vector<array_loop_t> tmpaloops(hdros.size());
            std::vector<element_loop_t> tmpeloops(hdros.size());
            std::vector<std::string> tmpnameos(hdros.size());
            for (size_t i = 0; i < hdros.size(); i++)
            {
                hdros[i] = hdri;
                hdros[i].set_compression(gta::none);
                hdros[i].set_components(hdri.component_type(comp_indices[i]), hdri.component_size(comp_indices[i]));
                hdros[i].component_taglist(0) = hdri.component_taglist(comp_indices[i]);
                tmpfilenames[i] = fio::mktempfile(&(tmpfiles[i]));
                tmpaloops[i].start("", tmpfilenames[i]);
                tmpaloops[i].start_element_loop(tmpeloops[i], hdri, hdros[i]);
            }
            // Write the GTA data to temporary files
            if (hdri.data_size() > 0)
            {
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, hdri, hdro);
                for (uintmax_t e = 0; e < hdri.elements(); e++)
                {
                    const void *element = element_loop.read();
                    if (hdros.size() > 0)
                    {
                        size_t out_index = 0;
                        size_t out_comp_offset = 0;
                        for (uintmax_t i = 0; i < hdri.components(); i++)
                        {
                            if (i == comp_indices[out_index])
                            {
                                tmpeloops[out_index].write(static_cast<const char *>(element) + out_comp_offset);
                                out_index++;
                            }
                            out_comp_offset += hdri.component_size(i);
                        }
                    }
                }
            }
            // Combine the GTA data to a single output stream
            for (size_t i = 0; i < hdros.size(); i++)
            {
                tmpaloops[i].finish();
                array_loop_t tmploop;
                tmploop.start(tmpfilenames[i], "");
                tmploop.write(hdros[i], nameos[i]);
                tmploop.copy_data(hdros[i], hdros[i]);
                tmploop.finish();
                fio::close(tmpfiles[i], tmpfilenames[i]);
                fio::remove(tmpfilenames[i]);
            }
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
