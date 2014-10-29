/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2014
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

#include <png.h>

#include <gta/gta.hpp>

#include "base/msg.h"
#include "base/blb.h"
#include "base/fio.h"
#include "base/opt.h"
#include "base/str.h"
#include "base/end.h"

#include "lib.h"


extern "C" void gtatool_from_png_help(void)
{
    msg::req_txt("from-png <input-file> [<output-file>]\n"
            "\n"
            "Converts PNG images to GTAs.\n"
            "The output will be 8-bit or 16-bit. Colors and gray scales will "
            "follow sRGB convention, and alpha (if present) will be linear.");
}

static std::string namei;

static void my_png_error(png_structp /* png_ptr */, png_const_charp error_msg)
{
    throw exc(namei + ": " + error_msg);
}

static void my_png_warning(png_structp /* png_ptr */, png_const_charp warning_msg)
{
    msg::wrn(namei + ": " + warning_msg);
}

extern "C" int gtatool_from_png(int argc, char *argv[])
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
        gtatool_from_png_help();
        return 0;
    }

    try
    {
        array_loop_t array_loop;
        array_loop.start(std::vector<std::string>(1, arguments[0]), arguments.size() == 2 ? arguments[1] : "");

        namei = arguments[0];

        FILE* pngfile = fio::open(namei, "r");
        png_byte header[8];
        fio::read(header, 8, 1, pngfile, namei);
        if (png_sig_cmp(header, 0, 8))
            throw exc(namei + ": not a PNG file");
        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_ptr)
            throw exc(namei + ": png_create_read_struct failed");
        png_set_error_fn(png_ptr, NULL, my_png_error, my_png_warning);
        png_set_user_limits(png_ptr, 0x7fffffffL, 0x7fffffffL);
        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
            throw exc(namei + ": png_create_info_struct failed");
        png_init_io(png_ptr, pngfile);
        png_set_sig_bytes(png_ptr, 8);
        png_set_gamma(png_ptr, 2.2, 0.45455);
        png_read_png(png_ptr, info_ptr,
                PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_PACKING
                | (endianness::endianness == endianness::big ? 0 : PNG_TRANSFORM_SWAP_ENDIAN),
                NULL);
        int width = png_get_image_width(png_ptr, info_ptr);
        int height = png_get_image_height(png_ptr, info_ptr);
        int channels = png_get_channels(png_ptr, info_ptr);
        png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
        png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);
        png_textp text_ptr;
        png_uint_32 num_text = png_get_text(png_ptr, info_ptr, &text_ptr, NULL);

        gta::header hdr;
        if (width < 1 || height < 1)
            throw exc(namei + ": invalid image dimensions");
        for (unsigned int i = 0; i < num_text; i++) {
            try {
                std::string key_utf8 = "PNG/";
                key_utf8 += str::convert(text_ptr[i].key, "ISO-8859-1", "UTF-8");
                std::string value_utf8 = str::convert(text_ptr[i].text, "ISO-8859-1", "UTF-8");
                hdr.global_taglist().set(key_utf8.c_str(), value_utf8.c_str());
            }
            catch (...) {
                // ignore tags that we cannot convert; they were invalid anyway
            }
        }
        hdr.set_dimensions(width, height);
        gta::type gta_type = (bit_depth <= 8 ? gta::uint8 : gta::uint16);
        if (channels == 1) {
            hdr.set_components(gta_type);
            hdr.component_taglist(0).set("INTERPRETATION", "SRGB/GRAY");
        } else if (channels == 2) {
            hdr.set_components(gta_type, gta_type);
            hdr.component_taglist(0).set("INTERPRETATION", "SRGB/GRAY");
            hdr.component_taglist(1).set("INTERPRETATION", "ALPHA");
        } else if (channels == 3) {
            hdr.set_components(gta_type, gta_type, gta_type);
            hdr.component_taglist(0).set("INTERPRETATION", "SRGB/RED");
            hdr.component_taglist(1).set("INTERPRETATION", "SRGB/GREEN");
            hdr.component_taglist(2).set("INTERPRETATION", "SRGB/BLUE");
        } else if (channels == 4) {
            hdr.set_components(gta_type, gta_type, gta_type, gta_type);
            hdr.component_taglist(0).set("INTERPRETATION", "SRGB/RED");
            hdr.component_taglist(1).set("INTERPRETATION", "SRGB/GREEN");
            hdr.component_taglist(2).set("INTERPRETATION", "SRGB/BLUE");
            hdr.component_taglist(3).set("INTERPRETATION", "ALPHA");
        } else {
            throw exc(namei + ": invalid number of channels");
        }

        std::string nameo;
        array_loop.write(hdr, nameo);
        element_loop_t element_loop;
        array_loop.start_element_loop(element_loop, gta::header(), hdr);
        for (int i = 0; i < height; i++)
            element_loop.write(row_pointers[i], width);
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fio::close(pngfile, namei);
        array_loop.finish();
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
