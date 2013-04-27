/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2012, 2013
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

#include <setjmp.h>

#include <jpeglib.h>

#include <gta/gta.hpp>

#include "base/msg.h"
#include "base/blb.h"
#include "base/fio.h"
#include "base/opt.h"

#include "lib.h"


extern "C" void gtatool_from_jpeg_help(void)
{
    msg::req_txt("from-jpeg <input-file> [<output-file>]\n"
            "\n"
            "Converts JPEG images to GTAs.");
}

struct my_error_mgr
{
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

static void my_error_exit(j_common_ptr cinfo)
{
    struct my_error_mgr* my_err = reinterpret_cast<struct my_error_mgr*>(cinfo->err);
    longjmp(my_err->setjmp_buffer, 1);
}

extern "C" int gtatool_from_jpeg(int argc, char *argv[])
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
        gtatool_from_jpeg_help();
        return 0;
    }

    try
    {
        array_loop_t array_loop;
        array_loop.start(std::vector<std::string>(1, arguments[0]), arguments.size() == 2 ? arguments[1] : "");

        std::string namei = arguments[0];

        struct jpeg_decompress_struct cinfo;
        struct my_error_mgr jerr;
        JSAMPROW jrow[1];

        gta::header hdr;
        std::string nameo;

        FILE* jpegfile = fio::open(namei, "r");

        cinfo.err = jpeg_std_error(&jerr.pub);
        jerr.pub.error_exit = my_error_exit;
        if (setjmp(jerr.setjmp_buffer))
        {
            /* JPEG has signaled an error */
            char message[JMSG_LENGTH_MAX];
            (cinfo.err->format_message)(reinterpret_cast<jpeg_common_struct*>(&cinfo), message);
            jpeg_destroy_decompress(&cinfo);
            fio::close(jpegfile);
            throw exc(namei + ": " + message);
        }

        jpeg_create_decompress(&cinfo);
        jpeg_stdio_src(&cinfo, jpegfile);
        jpeg_read_header(&cinfo, TRUE);

        if (cinfo.image_width < 1 || cinfo.image_height < 1)
            throw exc(namei + ": invalid image dimensions");
        hdr.set_dimensions(cinfo.image_width, cinfo.image_height);

        if (cinfo.num_components != 1 && cinfo.num_components != 3)
            throw exc(namei + ": invalid number of components");
        if (cinfo.num_components == 1) {
            hdr.set_components(gta::uint8);
            hdr.component_taglist(0).set("INTERPRETATION", "GRAY");
        } else {
            hdr.set_components(gta::uint8, gta::uint8, gta::uint8);
            hdr.component_taglist(0).set("INTERPRETATION", "SRGB/RED");
            hdr.component_taglist(1).set("INTERPRETATION", "SRGB/GREEN");
            hdr.component_taglist(2).set("INTERPRETATION", "SRGB/BLUE");
        }

        array_loop.write(hdr, nameo);
        element_loop_t element_loop;
        array_loop.start_element_loop(element_loop, gta::header(), hdr);

        jpeg_start_decompress(&cinfo);
        blob row(cinfo.output_width, cinfo.num_components);
        jrow[0] = row.ptr<unsigned char>();

        while (cinfo.output_scanline < cinfo.image_height)
        {
            jpeg_read_scanlines(&cinfo, jrow, 1);
            element_loop.write(row.ptr(), cinfo.image_width);
        }

        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        fio::close(jpegfile, namei);

        array_loop.finish();
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
