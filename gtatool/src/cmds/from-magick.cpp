/*
 * from-magick.cpp
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

#include <wand/MagickWand.h>

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "cio.h"
#include "opt.h"
#include "debug.h"


extern "C" void gtatool_from_magick_help(void)
{
    msg::req_txt("from-magick [--force-format=l|la|rgb|rgba] <input-file> [<output-file>]\n"
            "\n"
            "Convers images readable by ImageMagick to GTAs.");
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

    FILE *fo = stdout;
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
        msg::err("%s", e.what());
        return 1;
    }

    MagickWandGenesis();
    MagickBooleanType magick_r;
    MagickWand *magick_wand;
    gta::header *hdr = NULL;
    gta::type channel_type = gta::uint8;
    bool has_alpha = false;
    bool is_graylevel = false;
    int retval = 0;

    /* Read image */
    magick_wand = NewMagickWand();
    magick_r = MagickReadImage(magick_wand, arguments[0].c_str());
    if (!magick_wand || magick_r == MagickFalse)
    {
        ExceptionType severity;
        char *description = MagickGetException(magick_wand, &severity);
        msg::err("ImageMagick error: %s", *description ? description : "unknown error");
        description = static_cast<char *>(MagickRelinquishMemory(description));
        magick_wand = DestroyMagickWand(magick_wand);
        MagickWandTerminus();
        return 1;
    }

    /* Create a GTA */
    try
    {
        hdr = new gta::header();
        if (format.value().empty())
        {
            has_alpha = (MagickGetImageType(magick_wand) == GrayscaleMatteType
                    || MagickGetImageType(magick_wand) == PaletteMatteType
                    || MagickGetImageType(magick_wand) == TrueColorMatteType
                    || MagickGetImageType(magick_wand) == ColorSeparationMatteType);
            is_graylevel = (MagickGetImageType(magick_wand) == BilevelType
                    || MagickGetImageType(magick_wand) == GrayscaleType
                    || MagickGetImageType(magick_wand) == GrayscaleMatteType);
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
        channel_type = (MagickGetImageDepth(magick_wand) <= 8 ? gta::uint8
                : MagickGetImageDepth(magick_wand) <= 16 ? gta::uint16
                : gta::float32);
        if (is_graylevel && !has_alpha)
        {
            hdr->set_components(channel_type);
            hdr->component_taglist(0).set("INTERPRETATION", "GRAY");
        }
        else if (is_graylevel && has_alpha)
        {
            hdr->set_components(channel_type, channel_type);
            hdr->component_taglist(0).set("INTERPRETATION", "GRAY");
            hdr->component_taglist(1).set("INTERPRETATION", "ALPHA");
        }
        else if (!is_graylevel && !has_alpha)
        {
            hdr->set_components(channel_type, channel_type, channel_type);
            hdr->component_taglist(0).set("INTERPRETATION", "RED");
            hdr->component_taglist(1).set("INTERPRETATION", "GREEN");
            hdr->component_taglist(2).set("INTERPRETATION", "BLUE");
        }
        else
        {
            hdr->set_components(channel_type, channel_type, channel_type, channel_type);
            hdr->component_taglist(0).set("INTERPRETATION", "RED");
            hdr->component_taglist(1).set("INTERPRETATION", "GREEN");
            hdr->component_taglist(2).set("INTERPRETATION", "BLUE");
            hdr->component_taglist(3).set("INTERPRETATION", "ALPHA");
        }
        hdr->set_dimensions(MagickGetImageWidth(magick_wand), MagickGetImageHeight(magick_wand));
        msg::inf("%d x %d array, %d element components of type %s",
                static_cast<int>(hdr->dimension_size(0)),
                static_cast<int>(hdr->dimension_size(1)),
                static_cast<int>(hdr->components()),
                (channel_type == gta::uint8 ? "uint8" : channel_type == gta::uint16 ? "uint16" : "float32"));
        // TODO: Extract as much meta data as possible and store it in global tags
        hdr->write_to(fo);
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        magick_wand = DestroyMagickWand(magick_wand);
        MagickWandTerminus();
        return 1;
    }

    /* Convert and write the data */
    bool magick_error = false;
    PixelIterator *magick_it = NewPixelIterator(magick_wand);
    if (!magick_it)
    {
        magick_error = true;
    }
    try
    {
        blob line(hdr->element_size(), hdr->dimension_size(0));
        gta::io_state so;
        for (uintmax_t y = 0; y < hdr->dimension_size(1) && !magick_error; y++)
        {
            unsigned long magick_width = hdr->dimension_size(0);
            PixelWand **magick_pixels = PixelGetNextIteratorRow(magick_it, &magick_width);
            if (!magick_pixels)
            {
                magick_error = true;
            }
            for (uintmax_t x = 0; x < hdr->dimension_size(0) && !magick_error; x++)
            {
                if (is_graylevel && !has_alpha)
                {
                    if (channel_type == gta::uint8)
                    {
                        uint8_t *l = line.ptr<uint8_t>();
                        l[x] = PixelGetRed(magick_pixels[x]) * 255.0;
                    }
                    else if (channel_type == gta::uint16)
                    {
                        uint16_t *l = line.ptr<uint16_t>();
                        l[x] = PixelGetRed(magick_pixels[x]) * 65535.0;
                    }
                    else
                    {
                        float *l = line.ptr<float>();
                        l[x] = PixelGetRed(magick_pixels[x]);
                    }
                }
                else if (is_graylevel && has_alpha)
                {
                    if (channel_type == gta::uint8)
                    {
                        uint8_t *l = line.ptr<uint8_t>();
                        l[2 * x + 0] = PixelGetRed(magick_pixels[x]) * 255.0;
                        l[2 * x + 1] = PixelGetAlpha(magick_pixels[x]) * 255.0;
                    }
                    else if (channel_type == gta::uint16)
                    {
                        uint16_t *l = line.ptr<uint16_t>();
                        l[2 * x + 0] = PixelGetRed(magick_pixels[x]) * 65535.0;
                        l[2 * x + 1] = PixelGetAlpha(magick_pixels[x]) * 65535.0;
                    }
                    else
                    {
                        float *l = line.ptr<float>();
                        l[2 * x + 0] = PixelGetRed(magick_pixels[x]);
                        l[2 * x + 1] = PixelGetAlpha(magick_pixels[x]);
                    }
                }
                else if (!is_graylevel && !has_alpha)
                {
                    if (channel_type == gta::uint8)
                    {
                        uint8_t *l = line.ptr<uint8_t>();
                        l[3 * x + 0] = PixelGetRed(magick_pixels[x]) * 255.0;
                        l[3 * x + 1] = PixelGetGreen(magick_pixels[x]) * 255.0;
                        l[3 * x + 2] = PixelGetBlue(magick_pixels[x]) * 255.0;
                    }
                    else if (channel_type == gta::uint16)
                    {
                        uint16_t *l = line.ptr<uint16_t>();
                        l[3 * x + 0] = PixelGetRed(magick_pixels[x]) * 65535.0;
                        l[3 * x + 1] = PixelGetGreen(magick_pixels[x]) * 65535.0;
                        l[3 * x + 2] = PixelGetBlue(magick_pixels[x]) * 65535.0;
                    }
                    else
                    {
                        float *l = line.ptr<float>();
                        l[3 * x + 0] = PixelGetRed(magick_pixels[x]);
                        l[3 * x + 1] = PixelGetGreen(magick_pixels[x]);
                        l[3 * x + 2] = PixelGetBlue(magick_pixels[x]);
                    }
                }
                else
                {
                    if (channel_type == gta::uint8)
                    {
                        uint8_t *l = line.ptr<uint8_t>();
                        l[4 * x + 0] = PixelGetRed(magick_pixels[x]) * 255.0;
                        l[4 * x + 1] = PixelGetGreen(magick_pixels[x]) * 255.0;
                        l[4 * x + 2] = PixelGetBlue(magick_pixels[x]) * 255.0;
                        l[4 * x + 3] = PixelGetAlpha(magick_pixels[x]) * 255.0;
                    }
                    else if (channel_type == gta::uint16)
                    {
                        uint16_t *l = line.ptr<uint16_t>();
                        l[4 * x + 0] = PixelGetRed(magick_pixels[x]) * 65535.0;
                        l[4 * x + 1] = PixelGetGreen(magick_pixels[x]) * 65535.0;
                        l[4 * x + 2] = PixelGetBlue(magick_pixels[x]) * 65535.0;
                        l[4 * x + 3] = PixelGetAlpha(magick_pixels[x]) * 65535.0;
                    }
                    else
                    {
                        float *l = line.ptr<float>();
                        l[4 * x + 0] = PixelGetRed(magick_pixels[x]);
                        l[4 * x + 1] = PixelGetGreen(magick_pixels[x]);
                        l[4 * x + 2] = PixelGetBlue(magick_pixels[x]);
                        l[4 * x + 3] = PixelGetAlpha(magick_pixels[x]);
                    }
                }
            }
            hdr->write_elements(so, fo, hdr->dimension_size(0), line.ptr());
        }
        if (magick_error)
        {
            ExceptionType severity;
            char *description = MagickGetException(magick_wand, &severity);
            msg::err("ImageMagick error: %s", *description ? description : "unknown error");
            description = static_cast<char *>(MagickRelinquishMemory(description));
            retval = 1;
        }
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        retval = 1;
    }

    if (magick_it)
    {
        magick_it = DestroyPixelIterator(magick_it);
    }
    magick_wand = DestroyMagickWand(magick_wand);
    if (fo != stdout)
    {
        cio::close(fo);
    }
    MagickWandTerminus();
    return retval;
}
