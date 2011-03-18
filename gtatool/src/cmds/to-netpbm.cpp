/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010, 2011
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

#include <string>
#include <limits>

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "cio.h"
#include "opt.h"
#include "intcheck.h"

#include "lib.h"

extern "C"
{
/* This header must come last because it contains so much junk that
 * it messes up other headers. */
#include <pam.h>
#undef max
}


extern "C" void gtatool_to_netpbm_help(void)
{
    msg::req_txt("to-netpbm [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to a suitable NetPBM format using libnetpbm.");
}

extern "C" int gtatool_to_netpbm(int argc, char *argv[])
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
        gtatool_to_netpbm_help();
        return 0;
    }

    FILE *fi = gtatool_stdin;
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
        msg::err_txt("%s", e.what());
        return 1;
    }

    try
    {
        FILE *fo = cio::open(ofilename, "w");
        while (cio::has_more(fi, ifilename))
        {
            gta::header hdr;
            hdr.read_from(fi);
            if (hdr.dimensions() != 2)
            {
                throw exc("cannot export " + ifilename + ": only two-dimensional arrays can be exported via NetPBM");
            }
            if (hdr.dimension_size(0) > static_cast<uintmax_t>(std::numeric_limits<int>::max())
                    || hdr.dimension_size(1) > static_cast<uintmax_t>(std::numeric_limits<int>::max()))
            {
                throw exc("cannot export " + ifilename + ": array too large");
            }
            if (hdr.components() < 1 || hdr.components() > 4)
            {
                throw exc("cannot export " + ifilename + ": only arrays with 1-4 element components can be exported via NetPBM");
            }
            gta::type type = hdr.component_type(0);
            if (type != gta::uint8 && type != gta::uint16 && type != gta::uint32 && type != gta::uint64)
            {
                throw exc("cannot export " + ifilename + ": only arrays with unsigned integer element components can be exported via NetPBM");
            }
            for (uintmax_t i = 1; i < hdr.components(); i++)
            {
                if (hdr.component_type(i) != type)
                {
                    throw exc("cannot export " + ifilename + ": only arrays with element components that have a single type can be exported via NetPBM");
                }
            }
            if (hdr.compression() != gta::none)
            {
                throw exc("cannot export " + ifilename + ": currently only uncompressed GTAs can be exported via NetPBM");
            }

            struct pam outpam;
            outpam.size = sizeof(struct pam);
            outpam.len = outpam.size;
            outpam.file = fo;
            outpam.width = hdr.dimension_size(0);
            outpam.height = hdr.dimension_size(1);
            outpam.depth = hdr.components();
            outpam.maxval = (type == gta::uint8 ? std::numeric_limits<uint8_t>::max()
                    : type == gta::uint16 ? std::numeric_limits<uint16_t>::max()
                    : type == gta::uint32 ? std::numeric_limits<uint32_t>::max()
                    : std::numeric_limits<uint64_t>::max());
            outpam.plainformat = 0;
            if (hdr.components() == 1)
            {
                outpam.format = RPGM_FORMAT;
            }
            else if (hdr.components() == 2)
            {
                outpam.format = PAM_FORMAT;
                strcpy(outpam.tuple_type, "GRAYSCALE_ALPHA");
            }
            else if (hdr.components() == 3)
            {
                outpam.format = RPPM_FORMAT;
            }
            else
            {
                outpam.format = PAM_FORMAT;
                strcpy(outpam.tuple_type, "RGB_ALPHA");
            }
            pnm_writepaminit(&outpam);

            tuple *tuplerow = pnm_allocpamrow(&outpam);
            blob dataline(checked_cast<size_t>(hdr.dimension_size(0)), checked_cast<size_t>(hdr.element_size()));
            gta::io_state si;
            for (uintmax_t y = 0; y < hdr.dimension_size(1); y++)
            {
                hdr.read_elements(si, fi, hdr.dimension_size(0), dataline.ptr());
                for (uintmax_t x = 0; x < hdr.dimension_size(0); x++)
                {
                    void *element = hdr.element(dataline.ptr(), x, 0);
                    for (uintmax_t i = 0; i < hdr.components(); i++)
                    {
                        void *component = hdr.component(element, i);
                        if (type == gta::uint8)
                        {
                            uint8_t v;
                            memcpy(&v, component, sizeof(uint8_t));
                            tuplerow[x][i] = v;
                        }
                        else if (type == gta::uint16)
                        {
                            uint16_t v;
                            memcpy(&v, component, sizeof(uint16_t));
                            tuplerow[x][i] = v;
                        }
                        else if (type == gta::uint32)
                        {
                            uint32_t v;
                            memcpy(&v, component, sizeof(uint32_t));
                            tuplerow[x][i] = v;
                        }
                        else
                        {
                            uint64_t v;
                            memcpy(&v, component, sizeof(uint64_t));
                            tuplerow[x][i] = v;
                        }
                    }
                }
                pnm_writepamrow(&outpam, tuplerow);
            }
            pnm_freepamrow(tuplerow);
        }
        cio::close(fo);
        if (fi != gtatool_stdin)
        {
            cio::close(fi);
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
