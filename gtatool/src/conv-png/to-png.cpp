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
#include <cstring>
#include <cstdlib>

#include <zlib.h>
#include <png.h>

#include <gta/gta.hpp>

#include "base/msg.h"
#include "base/blb.h"
#include "base/fio.h"
#include "base/opt.h"
#include "base/str.h"
#include "base/end.h"

#include "lib.h"


extern "C" void gtatool_to_png_help(void)
{
    msg::req_txt("to-png [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to PNG image file format via libpng.\n"
            "This will produce PNGs with one of the formats GRAY, GRAY+ALPHA, RGB, or RGB+ALPHA, "
            "depending on the number of element components in the input array.\n"
            "It is assumed that the array components are in the correct order and "
            "contain sRGB data, and are of type uint8 or uint16. If this is not the "
            "case, use component-convert, component-reorder, and/or component-compute "
            "to prepare your array.");
}

static std::string nameo;

static void my_png_error(png_structp /* png_ptr */, png_const_charp error_msg)
{
    throw exc(nameo + ": " + error_msg);
}

static void my_png_warning(png_structp /* png_ptr */, png_const_charp warning_msg)
{
    msg::wrn(nameo + ": " + warning_msg);
}

extern "C" int gtatool_to_png(int argc, char *argv[])
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
        gtatool_to_png_help();
        return 0;
    }

    try
    {
        nameo = arguments.size() == 1 ? arguments[0] : arguments[1];
        array_loop_t array_loop;
        gta::header hdr;
        std::string name;

        array_loop.start(arguments.size() == 1 ? std::vector<std::string>() : std::vector<std::string>(1, arguments[0]), nameo);
        if (array_loop.read(hdr, name))
        {
            if (hdr.dimensions() != 2)
                throw exc(name + ": only two-dimensional arrays can be converted to PNG.");
            if (hdr.dimension_size(0) > 0x7fffffffL || hdr.dimension_size(1) > 0x7fffffffL)
                throw exc(name + ": array too large to be converted to PNG.");
            if (hdr.components() < 1 && hdr.components() > 4)
                throw exc(name + ": only arrays with 1-4 element components can be converted to PNG.");
            for (uintmax_t i = 0; i < hdr.components(); i++)
                if (hdr.component_type(i) != gta::uint8 && hdr.component_type(i) != gta::uint16)
                    throw exc(name + ": only arrays with element component type uint8 or uint16 can be converted to PNG.");
            for (uintmax_t i = 1; i < hdr.components(); i++)
                if (hdr.component_type(i) != hdr.component_type(0))
                    throw exc(name + ": only arrays with uniform element component types can be converted to PNG.");

            blob data(checked_cast<size_t>(hdr.data_size()));
            std::vector<png_bytep> row_pointers(hdr.dimension_size(1));
            size_t row_size = hdr.dimension_size(0) * hdr.element_size();
            for (uintmax_t i = 0; i < hdr.dimension_size(1); i++)
                row_pointers[i] = data.ptr<png_byte>(i * row_size);
            std::vector<struct png_text_struct> text;
            for (uintmax_t i = 0; i < hdr.global_taglist().tags(); i++) {
                std::string key = hdr.global_taglist().name(i);
                bool add = true;
                if (key.substr(0, 4) == std::string("PNG/"))
                    key = key.substr(4);
                std::string key_latin1, value_latin1;
                if (key.length() > 79) {
                    add = false;
                } else {
                    try {
                        key_latin1 = str::convert(key, "UTF-8", "ISO-8859-1");
                        value_latin1 = (hdr.global_taglist().value(i)
                                ? str::convert(hdr.global_taglist().value(i), "UTF-8", "ISO-8859-1")
                                : std::string(""));
                    }
                    catch (...) {
                        add = false;
                    }
                }
                if (add) {
                    struct png_text_struct t;
                    std::memset(&t, 0, sizeof(t));
                    t.compression = -1;
                    t.key = strdup(key_latin1.c_str());
                    t.text = strdup(value_latin1.c_str());
                    t.text_length = std::strlen(t.text);
                    text.push_back(t);
                    if (!t.key || !t.text)
                        throw exc(nameo + ": memory allocation failed");
                } else {
                    // silently ignore tags that we cannot set for PNG
                }
            }

            FILE* pngfile = fio::open(nameo, "w");
            png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            if (!png_ptr)
                throw exc(nameo + ": png_create_write_struct failed");
            png_set_error_fn(png_ptr, NULL, my_png_error, my_png_warning);
            png_set_user_limits(png_ptr, 0x7fffffffL, 0x7fffffffL);
            png_infop info_ptr = png_create_info_struct(png_ptr);
            if (!info_ptr)
                throw exc(nameo + ": png_create_info_struct failed");
            png_init_io(png_ptr, pngfile);
            png_set_IHDR(png_ptr, info_ptr,
                    hdr.dimension_size(0), hdr.dimension_size(1),
                    hdr.component_type(0) == gta::uint8 ? 8 : 16,
                    hdr.components() == 1 ? PNG_COLOR_TYPE_GRAY
                    : hdr.components() == 2 ? PNG_COLOR_TYPE_GRAY_ALPHA
                    : hdr.components() == 3 ? PNG_COLOR_TYPE_RGB
                    : PNG_COLOR_TYPE_RGB_ALPHA,
                    PNG_INTERLACE_NONE,
                    PNG_COMPRESSION_TYPE_DEFAULT,
                    PNG_FILTER_TYPE_DEFAULT);
            png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);
            png_set_sRGB(png_ptr, info_ptr, PNG_INFO_sRGB);
            if (text.size() > 0)
                png_set_text(png_ptr, info_ptr, &(text[0]), text.size());
            png_set_rows(png_ptr, info_ptr, &(row_pointers[0]));

            array_loop.read_data(hdr, data.ptr());
            png_write_png(png_ptr, info_ptr,
                    endianness::endianness == endianness::big
                    ? PNG_TRANSFORM_IDENTITY : PNG_TRANSFORM_SWAP_ENDIAN,
                    NULL);
            png_destroy_write_struct(&png_ptr, &info_ptr);
            for (size_t i = 0; i < text.size(); i++) {
                std::free(text[i].key);
                std::free(text[i].text);
            }

            fio::close(pngfile, nameo);
        }
        array_loop.finish();
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
