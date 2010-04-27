/*
 * from-netpbm.cpp
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

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "cio.h"
#include "opt.h"
#include "debug.h"

extern "C"
{
/* This header must come last because it contains so much junk that
 * it messes up other headers. */
#include <pam.h>
}


extern "C" void gtatool_from_netpbm_help(void)
{
    msg::req_txt("from-netpbm <input-file> [<output-file>]\n"
            "\n"
            "Converts NetPBM images to GTAs using libnetpbm.");
}

extern "C" int gtatool_from_netpbm(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, 2, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_from_netpbm_help();
        return 0;
    }

    FILE *fo = stdout;
    std::string ifilename(arguments[0]);
    std::string ofilename("standard output");
    try
    {
        if (arguments.size() == 2)
        {
            ofilename = arguments[1];
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
        FILE *fi = cio::open(ifilename, "r");
        while (cio::has_more(fi, ifilename))
        {
            // GTA
            gta::header hdr;
            blob dataline;
            // NetPBM
            struct pam inpam;
            tuple *tuplerow;

            pm_init("gta from-netpbm", 0);
#ifdef PAM_STRUCT_SIZE
            pnm_readpaminit(fi, &inpam, PAM_STRUCT_SIZE(tuple_type));
#else
            pnm_readpaminit(fi, &inpam, sizeof(struct pam));
#endif

            if (inpam.width < 1 || inpam.height < 1 || inpam.depth < 1)
            {
                throw exc("Cannot import " + ifilename, "unsupported image dimensions");
            }
            if (inpam.bytes_per_sample != 1 && inpam.bytes_per_sample != 2
                    && inpam.bytes_per_sample != 4 && inpam.bytes_per_sample != 8)
            {
                throw exc("Cannot import " + ifilename, "unsupported number of bytes per sample");
            }

            hdr.set_dimensions(inpam.width, inpam.height);
            hdr.dimension_taglist(0).set("INTERPRETATION", "X");
            hdr.dimension_taglist(1).set("INTERPRETATION", "Y");
            uintmax_t components = inpam.depth;
            gta::type type = (inpam.bytes_per_sample == 1 ? gta::uint8
                    : inpam.bytes_per_sample == 2 ? gta::uint16
                    : inpam.bytes_per_sample == 4 ? gta::uint32 : gta::uint64);
            std::vector<gta::type> types(inpam.depth);
            for (uintmax_t i = 0; i < components; i++)
            {
                types[i] = type;
            }
            hdr.set_components(components, &(types[0]));
            if (components == 1)
            {
                hdr.component_taglist(0).set("INTERPRETATION", "GRAY");
            }
            else if (components == 2)
            {
                hdr.component_taglist(0).set("INTERPRETATION", "GRAY");
                hdr.component_taglist(1).set("INTERPRETATION", "ALPHA");
            }
            else if (components == 3)
            {
                hdr.component_taglist(0).set("INTERPRETATION", "RED");
                hdr.component_taglist(1).set("INTERPRETATION", "GREEN");
                hdr.component_taglist(2).set("INTERPRETATION", "BLUE");
            }
            else if (components == 4)
            {
                hdr.component_taglist(0).set("INTERPRETATION", "RED");
                hdr.component_taglist(1).set("INTERPRETATION", "GREEN");
                hdr.component_taglist(2).set("INTERPRETATION", "BLUE");
                hdr.component_taglist(3).set("INTERPRETATION", "ALPHA");
            }

            tuplerow = pnm_allocpamrow(&inpam);
            dataline.resize(hdr.dimension_size(0), hdr.element_size());
            hdr.write_to(fo);
            gta::io_state so;
            for (uintmax_t y = 0; y < hdr.dimension_size(1); y++)
            {
                pnm_readpamrow(&inpam, tuplerow);
                for (uintmax_t x = 0; x < hdr.dimension_size(0); x++)
                {
                    void *element = hdr.element(dataline.ptr(), x, 0);
                    for (uintmax_t i = 0; i < components; i++)
                    {
                        void *component = hdr.component(element, i);
                        if (type == gta::uint8)
                        {
                            uint8_t v = tuplerow[x][i];
                            memcpy(component, &v, sizeof(uint8_t));
                        }
                        else if (type == gta::uint16)
                        {
                            uint16_t v = tuplerow[x][i];
                            memcpy(component, &v, sizeof(uint16_t));
                        }
                        else if (type == gta::uint32)
                        {
                            uint32_t v = tuplerow[x][i];
                            memcpy(component, &v, sizeof(uint32_t));
                        }
                        else
                        {
                            uint64_t v = tuplerow[x][i];
                            memcpy(component, &v, sizeof(uint64_t));
                        }
                    }
                }
                hdr.write_elements(so, fo, hdr.dimension_size(0), dataline.ptr());
            }
            pnm_freepamrow(tuplerow);
        }
        cio::close(fi);
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
