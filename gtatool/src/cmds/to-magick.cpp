/*
 * to-magick.cpp
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
#include <limits>

#include <wand/MagickWand.h>

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "cio.h"
#include "opt.h"
#include "debug.h"


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

    FILE *fi = stdin;
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
            fi = cio::open(filename, "r");
            magick_filename = arguments[1];
        }
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        return 1;
    }

    /* Read GTA header */
    gta::header *hdr = NULL;
    try
    {
        hdr = new gta::header();
        hdr->read_from(fi);
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        return 1;
    }
    if (hdr->dimensions() != 2)
    {
        msg::err("Only two-dimensional arrays can be exported to images");
        return 1;
    }
    if (hdr->components() < 1 || hdr->components() > 4)
    {
        msg::err("Only arrays with 1-4 element components can be exported to images");
        return 1;
    }
    gta::type channel_type = hdr->component_type(0);
    if (channel_type != gta::uint8 && channel_type != gta::uint16 && channel_type != gta::float32)
    {
        msg::err("Only arrays with element component types uint8, uint16, or float32 can be exported to images");
        return 1;
    }
    for (uintmax_t i = 1; i < hdr->components(); i++)
    {
        if (hdr->component_type(i) != channel_type)
        {
            msg::err("Only arrays with element components that all have the same type can be exported to images");
            return 1;
        }
    }
    if (hdr->data_is_chunked() && hdr->data_size() > std::numeric_limits<size_t>::max())
    {
        msg::err("Array too large");
        return 1;
    }
    bool has_alpha = (hdr->components() == 2 || hdr->components() == 4);
    bool is_graylevel = (hdr->components() <= 2);

    /* Create image */
    MagickWandGenesis();
    MagickWand *magick_wand = NewMagickWand();
    PixelWand *magick_pixel_wand = NewPixelWand();
    if (!magick_wand || !magick_pixel_wand
            || PixelSetColor(magick_pixel_wand, "none") == MagickFalse
            || MagickNewImage(magick_wand, hdr->dimension_size(0), hdr->dimension_size(1), magick_pixel_wand) == MagickFalse
            || MagickSetImageType(magick_wand,
                ((is_graylevel && !has_alpha) ? GrayscaleType
                 : (is_graylevel && has_alpha) ? GrayscaleMatteType
                 : (!is_graylevel && !has_alpha) ? TrueColorType
                 : TrueColorMatteType)) == MagickFalse
            || MagickSetImageChannelDepth(magick_wand, AllChannels,
                (channel_type == gta::uint8 ? 8 : channel_type == gta::uint16 ? 16 : 32)) == MagickFalse
            || MagickSetImageDepth(magick_wand,
                (channel_type == gta::uint8 ? 8 : channel_type == gta::uint16 ? 16 : 32)) == MagickFalse)
    {
        ExceptionType severity;
        char *description = MagickGetException(magick_wand, &severity);
        msg::err("ImageMagick error: %s", *description ? description : "unknown error");
        description = static_cast<char *>(MagickRelinquishMemory(description));
        magick_wand = DestroyMagickWand(magick_wand);
        MagickWandTerminus();
        return 1;
    }

    /* Convert the data */
    uintmax_t component_gray = 4;
    uintmax_t component_alpha = 4;
    uintmax_t component_red = 4;
    uintmax_t component_green = 4;
    uintmax_t component_blue = 4;
    if (is_graylevel && !has_alpha)
    {
        component_gray = 0;
    }
    else if (is_graylevel && has_alpha)
    {
        component_gray = 0;
        component_alpha = 1;
        for (uintmax_t i = 0; i < hdr->components(); i++)
        {
            const char *val = hdr->component_taglist(i).get("INTERPRETATION");
            if (val && strcmp(val, "LUMINANCE") == 0)
                component_gray = i;
            else if (val && strcmp(val, "ALPHA") == 0)
                component_alpha = i;
        }
    }
    else if (!is_graylevel && !has_alpha)
    {
        component_red = 0;
        component_green = 1;
        component_blue = 2;
        for (uintmax_t i = 0; i < hdr->components(); i++)
        {
            const char *val = hdr->component_taglist(i).get("INTERPRETATION");
            if (val && strstr(val, "RED") != 0)
                component_red = i;
            else if (val && strstr(val, "GREEN") != 0)
                component_green = i;
            else if (val && strstr(val, "BLUE") != 0)
                component_blue = i;
        }
    }
    else
    {
        component_red = 0;
        component_green = 1;
        component_blue = 2;
        component_alpha = 3;
        for (uintmax_t i = 0; i < hdr->components(); i++)
        {
            const char *val = hdr->component_taglist(i).get("INTERPRETATION");
            if (val && strstr(val, "RED") != 0)
                component_red = i;
            else if (val && strstr(val, "GREEN") != 0)
                component_green = i;
            else if (val && strstr(val, "BLUE") != 0)
                component_blue = i;
            else if (val && strcmp(val, "ALPHA") == 0)
                component_alpha = i;
        }
    }

    /* Convert and write the data */
    blob data;
    bool magick_error = false;
    PixelIterator *magick_it = NewPixelIterator(magick_wand);
    if (!magick_it)
    {
        magick_error = true;
    }
    try
    {
        if (hdr->data_is_chunked())
        {
            data.resize(hdr->data_size());
            hdr->read_data(fi, data.ptr());
        }
        else
        {
            data.resize(hdr->element_size(), hdr->dimension_size(0));
        }
        void *line = data.ptr();
        gta::io_state si;
        for (uintmax_t y = 0; y < hdr->dimension_size(1) && !magick_error; y++)
        {
            if (!hdr->data_is_chunked())
            {
                hdr->read_elements(si, fi, hdr->dimension_size(0), line);
            }
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
                        uint8_t *l = static_cast<uint8_t *>(line);
                        PixelSetRed(magick_pixels[x], static_cast<float>(l[x]) / 255.0f);
                        PixelSetGreen(magick_pixels[x], static_cast<float>(l[x]) / 255.0f);
                        PixelSetBlue(magick_pixels[x], static_cast<float>(l[x]) / 255.0f);
                    }
                    else if (channel_type == gta::uint16)
                    {
                        uint16_t *l = static_cast<uint16_t *>(line);
                        PixelSetRed(magick_pixels[x], static_cast<float>(l[x]) / 65535.0f);
                        PixelSetGreen(magick_pixels[x], static_cast<float>(l[x]) / 65535.0f);
                        PixelSetBlue(magick_pixels[x], static_cast<float>(l[x]) / 65535.0f);
                    }
                    else
                    {
                        float *l = static_cast<float *>(line);
                        PixelSetRed(magick_pixels[x], l[x]);
                        PixelSetGreen(magick_pixels[x], l[x]);
                        PixelSetBlue(magick_pixels[x], l[x]);
                    }
                }
                else if (is_graylevel && has_alpha)
                {
                    if (channel_type == gta::uint8)
                    {
                        uint8_t *l = static_cast<uint8_t *>(line);
                        PixelSetRed(magick_pixels[x], static_cast<float>(l[2 * x + component_gray]) / 255.0f);
                        PixelSetGreen(magick_pixels[x], static_cast<float>(l[2 * x + component_gray]) / 255.0f);
                        PixelSetBlue(magick_pixels[x], static_cast<float>(l[2 * x + component_gray]) / 255.0f);
                        PixelSetAlpha(magick_pixels[x], static_cast<float>(l[2 * x + component_alpha]) / 255.0f);
                    }
                    else if (channel_type == gta::uint16)
                    {
                        uint16_t *l = static_cast<uint16_t *>(line);
                        PixelSetRed(magick_pixels[x], static_cast<float>(l[2 * x + component_gray]) / 65535.0f);
                        PixelSetGreen(magick_pixels[x], static_cast<float>(l[2 * x + component_gray]) / 65535.0f);
                        PixelSetBlue(magick_pixels[x], static_cast<float>(l[2 * x + component_gray]) / 65535.0f);
                        PixelSetAlpha(magick_pixels[x], static_cast<float>(l[2 * x + component_alpha]) / 65535.0f);
                    }
                    else
                    {
                        float *l = static_cast<float *>(line);
                        PixelSetRed(magick_pixels[x], l[2 * x + component_gray]);
                        PixelSetGreen(magick_pixels[x], l[2 * x + component_gray]);
                        PixelSetBlue(magick_pixels[x], l[2 * x + component_gray]);
                        PixelSetAlpha(magick_pixels[x], l[2 * x + component_alpha]);
                    }
                }
                else if (!is_graylevel && !has_alpha)
                {
                    if (channel_type == gta::uint8)
                    {
                        uint8_t *l = static_cast<uint8_t *>(line);
                        PixelSetRed(magick_pixels[x], static_cast<float>(l[3 * x + component_red]) / 255.0f);
                        PixelSetGreen(magick_pixels[x], static_cast<float>(l[3 * x + component_green]) / 255.0f);
                        PixelSetBlue(magick_pixels[x], static_cast<float>(l[3 * x + component_blue]) / 255.0f);
                    }
                    else if (channel_type == gta::uint16)
                    {
                        uint16_t *l = static_cast<uint16_t *>(line);
                        PixelSetRed(magick_pixels[x], static_cast<float>(l[3 * x + component_red]) / 65535.0f);
                        PixelSetGreen(magick_pixels[x], static_cast<float>(l[3 * x + component_green]) / 65535.0f);
                        PixelSetBlue(magick_pixels[x], static_cast<float>(l[3 * x + component_blue]) / 65535.0f);
                    }
                    else
                    {
                        float *l = static_cast<float *>(line);
                        PixelSetRed(magick_pixels[x], l[3 * x + component_red]);
                        PixelSetGreen(magick_pixels[x], l[3 * x + component_green]);
                        PixelSetBlue(magick_pixels[x], l[3 * x + component_blue]);
                    }
                }
                else
                {
                    if (channel_type == gta::uint8)
                    {
                        uint8_t *l = static_cast<uint8_t *>(line);
                        PixelSetRed(magick_pixels[x], static_cast<float>(l[4 * x + component_red]) / 255.0f);
                        PixelSetGreen(magick_pixels[x], static_cast<float>(l[4 * x + component_green]) / 255.0f);
                        PixelSetBlue(magick_pixels[x], static_cast<float>(l[4 * x + component_blue]) / 255.0f);
                        PixelSetAlpha(magick_pixels[x], static_cast<float>(l[4 * x + component_alpha]) / 255.0f);
                    }
                    else if (channel_type == gta::uint16)
                    {
                        uint16_t *l = static_cast<uint16_t *>(line);
                        PixelSetRed(magick_pixels[x], static_cast<float>(l[4 * x + component_red]) / 65535.0f);
                        PixelSetGreen(magick_pixels[x], static_cast<float>(l[4 * x + component_green]) / 65535.0f);
                        PixelSetBlue(magick_pixels[x], static_cast<float>(l[4 * x + component_blue]) / 65535.0f);
                        PixelSetAlpha(magick_pixels[x], static_cast<float>(l[4 * x + component_alpha]) / 65535.0f);
                    }
                    else
                    {
                        float *l = static_cast<float *>(line);
                        PixelSetRed(magick_pixels[x], l[4 * x + component_red]);
                        PixelSetGreen(magick_pixels[x], l[4 * x + component_green]);
                        PixelSetBlue(magick_pixels[x], l[4 * x + component_blue]);
                        PixelSetAlpha(magick_pixels[x], l[4 * x + component_alpha]);
                    }
                }
            }
            (void)PixelSyncIterator(magick_it);
            if (hdr->data_is_chunked())
            {
                line = static_cast<char *>(line) + hdr->element_size() * hdr->dimension_size(0);
            }
        }
    }
    catch (std::exception &e)
    {
        msg::err("%s", e.what());
        return 1;
    }

    if (!magick_error)
    {
        // FIXME: this crashes when ImageMagick does not recognize the file extension!
        // FIXME: this always write 16 bpp RGB images, regardless of our settings!
        magick_error = (MagickWriteImage(magick_wand, magick_filename.c_str()) == MagickFalse);
    }

    if (magick_error)
    {
        ExceptionType severity;
        char *description = MagickGetException(magick_wand, &severity);
        msg::err("ImageMagick error: %s", *description ? description : "unknown error");
        description = static_cast<char *>(MagickRelinquishMemory(description));
        return 1;
    }
    magick_wand = DestroyMagickWand(magick_wand);
    if (magick_it)
    {
        magick_it = DestroyPixelIterator(magick_it);
    }

    if (fi != stdin)
    {
        cio::close(fi);
    }
    MagickWandTerminus();
    return 0;
}
