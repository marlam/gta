/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010, 2011, 2012, 2013
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

#include <ImfChannelList.h>
#include <ImfOutputFile.h>

#include <gta/gta.hpp>

#include "base/msg.h"
#include "base/blb.h"
#include "base/fio.h"
#include "base/opt.h"
#include "base/str.h"
#include "base/chk.h"

#include "lib.h"

using namespace Imf;
using namespace Imath;


extern "C" void gtatool_to_exr_help(void)
{
    msg::req_txt("to-exr [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to EXR format using OpenEXR.");
}

extern "C" int gtatool_to_exr(int argc, char *argv[])
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
        gtatool_to_exr_help();
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
            fi = fio::open(ifilename, "r");
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
        gta::header hdr;
        hdr.read_from(fi);
        if (hdr.dimensions() != 2)
        {
            throw exc("cannot export " + ifilename + ": only two-dimensional arrays can be exported to images");
        }
        if (hdr.components() < 1 || hdr.components() > 4)
        {
            throw exc("cannot export " + ifilename + ": only arrays with 1-4 element components can be exported to images");
        }
        for (uintmax_t i = 0; i < hdr.components(); i++)
        {
            if (hdr.component_type(i) == gta::int32
                    || hdr.component_type(i) == gta::uint32
                    || hdr.component_type(i) == gta::int64
                    || hdr.component_type(i) == gta::uint64
                    || hdr.component_type(i) == gta::float64)
            {
                msg::wrn_txt(std::string("converting ")
                        + (hdr.component_type(i) == gta::int32 ? "int32"
                         : hdr.component_type(i) == gta::uint32 ? "uint32"
                         : hdr.component_type(i) == gta::int64 ? "int64"
                         : hdr.component_type(i) == gta::uint64 ? "uint64" : "float64")
                        + " to float32 for array element component " + str::from(i) + " may lose precision");
            }
            else if (hdr.component_type(i) != gta::int8
                    && hdr.component_type(i) != gta::uint8
                    && hdr.component_type(i) != gta::int16
                    && hdr.component_type(i) != gta::uint16
                    && hdr.component_type(i) != gta::float32)
            {
                    throw exc("cannot export " + ifilename + ": array contains unexportable element component types");
            }
        }
        if (hdr.dimension_size(0) > static_cast<uintmax_t>(std::numeric_limits<int>::max())
                || hdr.dimension_size(1) > static_cast<uintmax_t>(std::numeric_limits<int>::max())
                || hdr.data_size() > std::numeric_limits<size_t>::max())
        {
            throw exc("cannot export " + ifilename + ": array too large");
        }
        blob data(checked_cast<size_t>(hdr.data_size()));
        hdr.read_data(fi, data.ptr());
        gta::header float_hdr;
        gta::type float_types[] = { gta::float32, gta::float32, gta::float32, gta::float32 };
        float_hdr.set_components(hdr.components(), float_types);
        float_hdr.set_dimensions(hdr.dimension_size(0), hdr.dimension_size(1));
        blob float_data(checked_cast<size_t>(float_hdr.data_size()));
        for (uintmax_t y = 0; y < hdr.dimension_size(1); y++)
        {
            for (uintmax_t x = 0; x < hdr.dimension_size(0); x++)
            {
                void *element = hdr.element(data.ptr(), x, y);
                void *float_element = float_hdr.element(float_data.ptr(), x, y);
                for (uintmax_t i = 0; i < hdr.components(); i++)
                {
                    void *component = hdr.component(element, i);
                    float *float_component = static_cast<float *>(float_hdr.component(float_element, i));
                    if (hdr.component_type(i) == gta::int8)
                    {
                        int8_t v;
                        memcpy(&v, component, sizeof(int8_t));
                        *float_component = v;
                    }
                    else if (hdr.component_type(i) == gta::uint8)
                    {
                        uint8_t v;
                        memcpy(&v, component, sizeof(uint8_t));
                        *float_component = v;
                    }
                    else if (hdr.component_type(i) == gta::int16)
                    {
                        int16_t v;
                        memcpy(&v, component, sizeof(int16_t));
                        *float_component = v;
                    }
                    else if (hdr.component_type(i) == gta::uint16)
                    {
                        uint16_t v;
                        memcpy(&v, component, sizeof(uint16_t));
                        *float_component = v;
                    }
                    else if (hdr.component_type(i) == gta::int32)
                    {
                        int32_t v;
                        memcpy(&v, component, sizeof(int32_t));
                        *float_component = v;
                    }
                    else if (hdr.component_type(i) == gta::uint32)
                    {
                        uint32_t v;
                        memcpy(&v, component, sizeof(uint32_t));
                        *float_component = v;
                    }
                    else if (hdr.component_type(i) == gta::int64)
                    {
                        int64_t v;
                        memcpy(&v, component, sizeof(int64_t));
                        *float_component = v;
                    }
                    else if (hdr.component_type(i) == gta::uint64)
                    {
                        uint64_t v;
                        memcpy(&v, component, sizeof(uint64_t));
                        *float_component = v;
                    }
                    else if (hdr.component_type(i) == gta::float64)
                    {
                        double v;
                        memcpy(&v, component, sizeof(double));
                        *float_component = v;
                    }
                    else // gta::float32
                    {
                        float v;
                        memcpy(&v, component, sizeof(float));
                        *float_component = v;
                    }
                }
            }
        }
        Header header(hdr.dimension_size(0), hdr.dimension_size(1), 1.0f, Imath::V2f(0, 0), 1.0f, INCREASING_Y, PIZ_COMPRESSION);
        std::string channel_names[4] = { "U0", "U1", "U2", "U3" };
        if (hdr.components() == 1)
        {
            channel_names[0] = "Y";
        }
        else if (hdr.components() == 2)
        {
            channel_names[0] = "Y";
            channel_names[1] = "A";
        }
        else if (hdr.components() == 3)
        {
            channel_names[0] = "R";
            channel_names[1] = "G";
            channel_names[2] = "B";
        }
        else
        {
            channel_names[0] = "R";
            channel_names[1] = "G";
            channel_names[2] = "B";
            channel_names[3] = "A";
        }
        for (uintmax_t c = 0; c < hdr.components(); c++)
        {
            header.channels().insert(channel_names[c].c_str(), Channel(FLOAT));
        }
        OutputFile file(ofilename.c_str(), header);
        FrameBuffer framebuffer;
        for (uintmax_t c = 0; c < hdr.components(); c++)
        {
            framebuffer.insert(channel_names[c].c_str(),
                    Slice(FLOAT, float_data.ptr<char>(c * sizeof(float)),
                        float_hdr.components() * sizeof(float),
                        float_hdr.components() * float_hdr.dimension_size(0) * sizeof(float)));
        }
        file.setFrameBuffer(framebuffer);
        file.writePixels(float_hdr.dimension_size(1));
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }
    catch (...)
    {
        msg::err_txt("Cannot write %s: OpenEXR error", ofilename.c_str());
        return 1;
    }

    return 0;
}
