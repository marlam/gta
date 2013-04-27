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
#include <ImfInputFile.h>

#include <gta/gta.hpp>

#include "base/msg.h"
#include "base/blb.h"
#include "base/fio.h"
#include "base/opt.h"
#include "base/chk.h"

#include "lib.h"

using namespace Imf;
using namespace Imath;


extern "C" void gtatool_from_exr_help(void)
{
    msg::req_txt("from-exr <input-file> [<output-file>]\n"
            "\n"
            "Converts EXR images to GTAs using OpenEXR.");
}

extern "C" int gtatool_from_exr(int argc, char *argv[])
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
        gtatool_from_exr_help();
        return 0;
    }

    FILE *fo = gtatool_stdout;
    std::string ifilename(arguments[0]);
    std::string ofilename("standard output");
    try
    {
        if (arguments.size() == 2)
        {
            ofilename = arguments[1];
            fo = fio::open(ofilename, "w");
        }
        if (fio::isatty(fo))
        {
            throw exc("refusing to write to a tty");
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    try
    {
        InputFile file(ifilename.c_str());
        Box2i dw = file.header().dataWindow();
        int width = dw.max.x - dw.min.x + 1;
        int height = dw.max.y - dw.min.y + 1;
        if (width < 1 || height < 1)
        {
            throw exc("cannot import " + ifilename + ": unsupported image dimensions");
        }
        const ChannelList &channellist = file.header().channels();
        uintmax_t channels = 0;
        for (ChannelList::ConstIterator iter = channellist.begin(); iter != channellist.end(); iter++)
        {
            channels++;
        }
        if (channels < 1 || channels > 4)
        {
            throw exc("cannot import " + ifilename + ": unsupported number of channels");
        }
        gta::header hdr;
        hdr.set_dimensions(width, height);
        hdr.dimension_taglist(0).set("INTERPRETATION", "X");
        hdr.dimension_taglist(1).set("INTERPRETATION", "Y");
        gta::type types[] = { gta::float32, gta::float32, gta::float32, gta::float32 };
        hdr.set_components(channels, types, NULL);
        if (hdr.data_size() > std::numeric_limits<size_t>::max())
        {
            throw exc("cannot import " + ifilename + ": image too large");
        }
        blob data(checked_cast<size_t>(hdr.data_size()));
        FrameBuffer framebuffer;
        int c = 0;
        for (ChannelList::ConstIterator iter = channellist.begin(); iter != channellist.end(); iter++)
        {
            framebuffer.insert(iter.name(), Slice(FLOAT, data.ptr<char>(c * sizeof(float)),
                        channels * sizeof(float), channels * width * sizeof(float), 1, 1, 0.0f));
            c++;
        }
        file.setFrameBuffer(framebuffer);
        file.readPixels(dw.min.y, dw.max.y);
        hdr.write_to(fo);
        hdr.write_data(fo, data.ptr());
        if (fo != gtatool_stdout)
        {
            fio::close(fo);
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }
    catch (...)
    {
        msg::err_txt("Cannot load %s: OpenEXR error", ifilename.c_str());
        return 1;
    }

    return 0;
}
