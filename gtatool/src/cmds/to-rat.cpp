/*
 * to-rat.cpp
 *
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2007, 2008, 2009, 2010  Martin Lambers <marlam@marlam.de>
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
#include "endianness.h"

#include "lib.h"


extern "C" void gtatool_to_rat_help(void)
{
    msg::req_txt("to-rat [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to RAT RadarTools files.");
}

static void reorder_rat_data(gta::header &dsthdr, void *dst, const gta::header &srchdr, const void *src)
{
    dsthdr = srchdr;
    std::vector<uintmax_t> dstindices(checked_cast<size_t>(dsthdr.dimensions()));
    std::vector<uintmax_t> srcindices(checked_cast<size_t>(srchdr.dimensions()));
    for (uintmax_t i = 0; i < dsthdr.elements(); i++)
    {
        // For arrays with 2 dimensions, we have to mirror the y component.
        // This code always mirrors the last component; I'm not sure that that's correct.
        dsthdr.linear_index_to_indices(i, &(dstindices[0]));
        for (uintmax_t j = 0; j < dsthdr.dimensions(); j++)
        {
            if (j == dsthdr.dimensions() - 1)
            {
                srcindices[j] = dsthdr.dimension_size(j) - 1 - dstindices[j];
            }
            else
            {
                srcindices[j] = dstindices[j];
            }
        }
        uintmax_t k = srchdr.indices_to_linear_index(&(srcindices[0]));
        memcpy(dsthdr.element(dst, i), srchdr.element(src, k), dsthdr.element_size());
        if (endianness::endianness == endianness::little)
        {
            swap_element_endianness(dsthdr, dsthdr.element(dst, i));
        }
    }
}

extern "C" int gtatool_to_rat(int argc, char *argv[])
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
        gtatool_to_rat_help();
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

        uintmax_t array_index = 0;
        while (cio::has_more(fi, ifilename))
        {
            std::string array_name = ifilename + " array " + str::from(array_index);
            gta::header ihdr;
            ihdr.read_from(fi);
            if (ihdr.components() != 1)
            {
                throw exc("cannot export " + array_name + ": only arrays with a single array element component can be exported to RAT RadarTools");
            }
            if (ihdr.dimensions() == 0)
            {
                msg::wrn(array_name + ": ignoring empty array");
                continue;
            }
            int32_t rat_var;
            switch (ihdr.component_type(0))
            {
            case gta::uint8:
                rat_var = 1;
                break;
            case gta::int32:
                rat_var = 2;
                break;
            case gta::uint32:
                rat_var = 12;
                break;
            case gta::int64:
                rat_var = 14;
                break;
            case gta::uint64:
                rat_var = 15;
                break;
            case gta::float32:
                rat_var = 4;
                break;
            case gta::float64:
                rat_var = 5;
                break;
            case gta::cfloat32:
                rat_var = 6;
                break;
            case gta::cfloat64:
                rat_var = 9;
                break;
            default:
                throw exc(std::string("type ") + type_to_string(ihdr.component_type(0), ihdr.component_size(0))
                        + " cannot be exported to RAT RadarTools");
                break;
            }

            blob idata(checked_cast<size_t>(ihdr.data_size()));
            cio::read(idata.ptr(), ihdr.data_size(), 1, fi, ifilename);

            gta::header hdr;
            blob data(checked_cast<size_t>(ihdr.data_size()));
            reorder_rat_data(hdr, data.ptr(), ihdr, idata.ptr());

            int32_t rat_dim = static_cast<int32_t>(hdr.dimensions());
            std::vector<int32_t> rat_sizes(rat_dim);
            for (int i = 0; i < rat_dim; i++)
            {
                rat_sizes[i] = checked_cast<int32_t>(hdr.dimension_size(i));
            }

            int32_t rat_type = 0;
            const char *rt = ihdr.global_taglist().get("RAT/TYPE");
            if (rt)
            {
                rat_type = str::to<int32_t>(rt);
            }
            int32_t rat_dummy[4] = { 0, 0, 0, 0x50 };   // I have no idea what these fields mean, but these values work.
            char rat_info[80];
            for (int i = 0; i < 80; i++)
            {
                rat_info[i] = ' ';
            }
            const char *info = ihdr.global_taglist().get("RAT/INFO");
            if (info)
            {
                size_t l = strlen(info);
                for (size_t i = 0; i < l && i < 80; i++)
                {
                    rat_info[i] = info[i];
                }
            }

            if (endianness::endianness == endianness::little)
            {
                endianness::swap32(&rat_dim);
                for (size_t i = 0; i < rat_sizes.size(); i++)
                {
                    endianness::swap32(&(rat_sizes[i]));
                }
                endianness::swap32(&rat_var);
                endianness::swap32(&rat_type);
                for (size_t i = 0; i < 4; i++)
                {
                    endianness::swap32(&(rat_dummy[i]));
                }
            }
            cio::write(&rat_dim, sizeof(int32_t), 1, fo, ofilename);
            cio::write(&(rat_sizes[0]), sizeof(int32_t), rat_sizes.size(), fo, ofilename);
            cio::write(&rat_var, sizeof(int32_t), 1, fo, ofilename);
            cio::write(&rat_type, sizeof(int32_t), 1, fo, ofilename);
            cio::write(rat_dummy, sizeof(int32_t), 4, fo, ofilename);
            cio::write(rat_info, sizeof(char), 80, fo, ofilename);
            cio::write(data.ptr(), hdr.data_size(), 1, fo, ofilename);
            array_index++;
        }
        cio::close(fo, ofilename);
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
