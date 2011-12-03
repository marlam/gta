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

#include <Magick++.h>

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "fio.h"
#include "opt.h"
#include "str.h"
#include "intcheck.h"

#include "lib.h"


extern "C" void gtatool_to_magick_help(void)
{
    msg::req_txt("to-magick [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to a format supported by ImageMagick, determined from "
            "the name of the output file.");
}

extern "C" int gtatool_to_magick(int argc, char *argv[])
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
        gtatool_to_magick_help();
        return 0;
    }

    FILE *fi = gtatool_stdin;
    std::string filename("standard input");
    std::string magick_filename;
    try
    {
        if (arguments.size() == 1)
        {
            magick_filename = arguments[0];
        }
        else
        {
            filename = arguments[0];
            fi = fio::open(filename, "r");
            magick_filename = arguments[1];
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
        uintmax_t array_index = 0;
        while (fio::has_more(fi, filename))
        {
            std::string array_name = filename + " array " + str::from(array_index);
            gta::header hdr;
            hdr.read_from(fi);
            if (hdr.dimensions() != 2)
            {
                throw exc(array_name + ": only two-dimensional arrays can be exported to images");
            }
            if (hdr.components() < 1 || hdr.components() > 4)
            {
                throw exc(array_name + ": only arrays with 1-4 element components can be exported to images");
            }
            gta::type channel_type = hdr.component_type(0);
            if (channel_type != gta::uint8 && channel_type != gta::uint16 && channel_type != gta::float32)
            {
                throw exc(array_name + ": only arrays with element component types uint8, uint16, or float32 can be exported to images");
            }
            for (uintmax_t i = 1; i < hdr.components(); i++)
            {
                if (hdr.component_type(i) != channel_type)
                {
                    throw exc(array_name + ": only arrays with element components that all have the same type can be exported to images");
                }
            }
            if (hdr.dimension_size(0) > std::numeric_limits<unsigned long>::max()
                    || hdr.dimension_size(1) > std::numeric_limits<unsigned long>::max()
                    || hdr.data_size() > std::numeric_limits<size_t>::max())
            {
                throw exc(array_name + ": array too large");
            }
            bool has_alpha = (hdr.components() == 2 || hdr.components() == 4);
            bool is_graylevel = (hdr.components() <= 2);
            std::string map;
            if (is_graylevel && !has_alpha)
            {
                map = "I";
            }
            else if (is_graylevel && has_alpha)
            {
                int component_gray = 0;
                int component_alpha = 1;
                for (uintmax_t i = 0; i < hdr.components(); i++)
                {
                    const char *val = hdr.component_taglist(i).get("INTERPRETATION");
                    if (val && strcmp(val, "LUMINANCE") == 0)
                        component_gray = i;
                    else if (val && strcmp(val, "ALPHA") == 0)
                        component_alpha = i;
                }
                map = "IA";
                if (component_gray != component_alpha)
                {
                    map[component_gray] = 'I';
                    map[component_alpha] = 'A';
                }
            }
            else if (!is_graylevel && !has_alpha)
            {
                int component_red = 0;
                int component_green = 1;
                int component_blue = 2;
                for (uintmax_t i = 0; i < hdr.components(); i++)
                {
                    const char *val = hdr.component_taglist(i).get("INTERPRETATION");
                    if (val && strstr(val, "RED") != 0)
                        component_red = i;
                    else if (val && strstr(val, "GREEN") != 0)
                        component_green = i;
                    else if (val && strstr(val, "BLUE") != 0)
                        component_blue = i;
                }
                map = "RGB";
                if (component_red != component_green
                        && component_red != component_blue
                        && component_green != component_blue)
                {
                    map[component_red] = 'R';
                    map[component_green] = 'G';
                    map[component_blue] = 'B';
                }
            }
            else
            {
                int component_red = 0;
                int component_green = 1;
                int component_blue = 2;
                int component_alpha = 3;
                for (uintmax_t i = 0; i < hdr.components(); i++)
                {
                    const char *val = hdr.component_taglist(i).get("INTERPRETATION");
                    if (val && strstr(val, "RED") != 0)
                        component_red = i;
                    else if (val && strstr(val, "GREEN") != 0)
                        component_green = i;
                    else if (val && strstr(val, "BLUE") != 0)
                        component_blue = i;
                    else if (val && strcmp(val, "ALPHA") == 0)
                        component_alpha = i;
                }
                map = "RGBA";
                if (component_red != component_green
                        && component_red != component_blue
                        && component_red != component_alpha
                        && component_green != component_blue
                        && component_green != component_alpha
                        && component_blue != component_alpha)
                {
                    map[component_red] = 'R';
                    map[component_green] = 'G';
                    map[component_blue] = 'B';
                    map[component_alpha] = 'A';
                }
            }
            Magick::StorageType storage_type = (channel_type == gta::uint8 ? Magick::CharPixel
                    : channel_type == gta::uint16 ? Magick::ShortPixel : Magick::FloatPixel);
            blob data(checked_cast<size_t>(hdr.data_size()));
            hdr.read_data(fi, data.ptr());
            imgs.push_back(Magick::Image());
            imgs.back().read(hdr.dimension_size(0), hdr.dimension_size(1), map.c_str(), storage_type, data.ptr());
            array_index++;
        }
        if (fi != gtatool_stdin)
        {
            fio::close(fi);
        }
        Magick::writeImages(imgs.begin(), imgs.end(), magick_filename);
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
