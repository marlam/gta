/*
 * component-merge.cpp
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
#include <vector>

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "opt.h"
#include "cio.h"
#include "str.h"

#include "lib.h"

#ifndef EOVERFLOW
#   define EOVERFLOW EFBIG
#endif


extern "C" void gtatool_component_merge_help(void)
{
    msg::req_txt(
            "component-merge <files>...\n"
            "\n"
            "Merges the array element components of the given GTAs, and writes "
            "the resulting GTA to standard output. The first file determines the "
            "dimensions and the global and dimension tag lists of the output. "
            "Component tag lists of all input files will be preserved.\n"
            "Example: component-merge r.gta g.gta b.gta > rgb.gta");
}

extern "C" int gtatool_component_merge(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_component_merge_help();
        return 0;
    }

    if (cio::isatty(stdout))
    {
        msg::err("refusing to write to a tty");
        return 1;
    }

    try
    {
        std::vector<FILE *> fi(arguments.size());
        for (size_t i = 0; i < arguments.size(); i++)
        {
            fi[i] = cio::open(arguments[i], "r");
        }
        std::vector<gta::header> hdri(arguments.size());
        while (cio::has_more(fi[0], arguments[0]))
        {
            for (size_t i = 0; i < arguments.size(); i++)
            {
                hdri[i].read_from(fi[i]);
                if (i > 0)
                {
                    if (hdri[i].dimensions() != hdri[0].dimensions())
                    {
                        throw exc(arguments[i] + ": incompatible GTA");
                    }                        
                    for (uintmax_t d = 0; d < hdri[0].dimensions(); d++)
                    {
                        if (hdri[i].dimension_size(d) != hdri[0].dimension_size(d))
                        {
                            throw exc(arguments[i] + ": incompatible GTA");
                        }
                    }
                }
            }
            gta::header hdro;
            hdro.global_taglist() = hdri[0].global_taglist();
            std::vector<uintmax_t> hdro_dim_sizes;
            for (uintmax_t d = 0; d < hdri[0].dimensions(); d++)
            {
                hdro_dim_sizes.push_back(hdri[0].dimension_size(d));
            }
            hdro.set_dimensions(hdro_dim_sizes.size(), &(hdro_dim_sizes[0]));
            for (uintmax_t d = 0; d < hdri[0].dimensions(); d++)
            {
                hdro.dimension_taglist(d) = hdri[0].dimension_taglist(d);
            }
            std::vector<gta::type> hdro_comp_types;
            std::vector<uintmax_t> hdro_comp_sizes;
            for (size_t i = 0; i < arguments.size(); i++)
            {
                for (uintmax_t c = 0; c < hdri[i].components(); c++)
                {
                    hdro_comp_types.push_back(hdri[i].component_type(c));
                    if (hdri[i].component_type(c) == gta::blob)
                    {
                        hdro_comp_sizes.push_back(hdri[i].component_size(c));
                    }
                }
            }
            hdro.set_components(hdro_comp_types.size(), &(hdro_comp_types[0]),
                    (hdro_comp_sizes.size() > 0 ? &(hdro_comp_sizes[0]) : NULL));
            if (hdro.element_size() > std::numeric_limits<size_t>::max())
            {
                throw exc("cannot merge components", EOVERFLOW);
            }
            uintmax_t hdro_c = 0;
            for (size_t i = 0; i < arguments.size(); i++)
            {
                for (uintmax_t c = 0; c < hdri[i].components(); c++)
                {
                    hdro.component_taglist(hdro_c) = hdri[i].component_taglist(c);
                    hdro_c++;
                }
            }
            hdro.write_to(stdout);
            blob element_buf(hdro.element_size());
            gta::io_state si, so;
            for (uintmax_t e = 0; e < hdro.elements(); e++)
            {
                void *e = element_buf.ptr();
                for (size_t i = 0; i < arguments.size(); i++)
                {
                    hdri[i].read_elements(si, fi[i], 1, e);
                    e = static_cast<void *>(static_cast<char *>(e) + hdri[i].element_size());
                }
                hdro.write_elements(so, stdout, 1, element_buf.ptr());
            }
        }
        for (size_t i = 1; i < arguments.size(); i++)
        {
            if (cio::has_more(fi[i], arguments[i]))
            {
                msg::wrn("ignoring additional GTA(s) from %s", arguments[i].c_str());
            }
        }
        for (size_t i = 0; i < arguments.size(); i++)
        {
            cio::close(fi[i], arguments[i]);
        }
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        return 1;
    }

    return 0;
}
