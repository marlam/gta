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
#include <list>

#include <Magick++.h>

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "cio.h"
#include "opt.h"
#include "intcheck.h"

#include "lib.h"


extern "C" void gtatool_from_magick_help(void)
{
    msg::req_txt("from-magick [--force-format=l|la|rgb|rgba] <input-file> [<output-file>]\n"
            "\n"
            "Converts images readable by ImageMagick to GTAs.");
}

extern "C" int gtatool_from_magick(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> formats;
    formats.push_back("l");
    formats.push_back("la");
    formats.push_back("rgb");
    formats.push_back("rgba");
    opt::val<std::string> format("force-format", '\0', opt::optional, formats);
    options.push_back(&format);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, 2, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_from_magick_help();
        return 0;
    }

    FILE *fo = gtatool_stdout;
    std::string filename("standard output");
    try
    {
        if (arguments.size() == 2)
        {
            filename = arguments[1];
            fo = cio::open(filename, "w");
        }
        if (cio::isatty(fo))
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
        std::vector<Magick::Image> imgs;
        Magick::readImages(&imgs, arguments[0].c_str());
        for (size_t i = 0; i < imgs.size(); i++)
        {
            gta::header hdr;
            gta::type channel_type = (imgs[i].channelDepth(Magick::RedChannel) <= 8 ? gta::uint8
                    : imgs[i].channelDepth(Magick::RedChannel) <= 16 ? gta::uint16
                    : gta::float32);        // XXX: this assumes that all channels have the same depth
            bool has_alpha = false;
            bool is_graylevel = false;
            if (format.value().empty())
            {
                has_alpha = imgs[i].matte();
                is_graylevel = (imgs[i].colorSpace() == Magick::GRAYColorspace);
            }
            else if (format.value().compare("l") == 0)
            {
                has_alpha = false;
                is_graylevel = true;
            }
            else if (format.value().compare("la") == 0)
            {
                has_alpha = true;
                is_graylevel = false;
            }
            else if (format.value().compare("rgb") == 0)
            {
                has_alpha = false;
                is_graylevel = false;
            }
            else
            {
                has_alpha = true;
                is_graylevel = false;
            }
            if (is_graylevel && !has_alpha)
            {
                hdr.set_components(channel_type);
                hdr.component_taglist(0).set("INTERPRETATION", "GRAY");
            }
            else if (is_graylevel && has_alpha)
            {
                hdr.set_components(channel_type, channel_type);
                hdr.component_taglist(0).set("INTERPRETATION", "GRAY");
                hdr.component_taglist(1).set("INTERPRETATION", "ALPHA");
            }
            else if (!is_graylevel && !has_alpha)
            {
                hdr.set_components(channel_type, channel_type, channel_type);
                hdr.component_taglist(0).set("INTERPRETATION", "RED");
                hdr.component_taglist(1).set("INTERPRETATION", "GREEN");
                hdr.component_taglist(2).set("INTERPRETATION", "BLUE");
            }
            else
            {
                hdr.set_components(channel_type, channel_type, channel_type, channel_type);
                hdr.component_taglist(0).set("INTERPRETATION", "RED");
                hdr.component_taglist(1).set("INTERPRETATION", "GREEN");
                hdr.component_taglist(2).set("INTERPRETATION", "BLUE");
                hdr.component_taglist(3).set("INTERPRETATION", "ALPHA");
            }
            hdr.set_dimensions(imgs[i].columns(), imgs[i].rows());
            msg::inf_txt("%d x %d array, %d element components of type %s",
                    static_cast<int>(hdr.dimension_size(0)),
                    static_cast<int>(hdr.dimension_size(1)),
                    static_cast<int>(hdr.components()),
                    (channel_type == gta::uint8 ? "uint8" : channel_type == gta::uint16 ? "uint16" : "float32"));
            //hdr.global_taglist().set("MAGICK/LABEL", imgs[i].label().c_str());
            //hdr.global_taglist().set("MAGICK/FORMAT", imgs[i].magick().c_str());
            // TODO: Extract as much meta data as possible and store it in global tags
            hdr.write_to(fo);
            /* Convert and write the data */
            blob data(checked_cast<size_t>(hdr.data_size()));
            Magick::StorageType storage_type = (channel_type == gta::uint8 ? Magick::CharPixel
                    : channel_type == gta::uint16 ? Magick::ShortPixel : Magick::FloatPixel);
            std::string map = (is_graylevel ? "I" : "RGB");
            if (has_alpha)
            {
                map += "A";
            }
            imgs[i].write(0, 0, hdr.dimension_size(0), hdr.dimension_size(1), map.c_str(), storage_type, data.ptr());
            hdr.write_data(fo, data.ptr());
        }
        if (fo != gtatool_stdout)
        {
            cio::close(fo);
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
