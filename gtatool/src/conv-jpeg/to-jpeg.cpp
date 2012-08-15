/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2012
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

#include "msg.h"
#include "blob.h"
#include "fio.h"
#include "opt.h"

#include "lib.h"


extern "C" void gtatool_to_jpeg_help(void)
{
    msg::req_txt("to-jpeg [-q|--quality=Q] [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to JPEG image file format via libjpeg. The quality Q is between 1 and 100; the default is 75.");
}

struct my_error_mgr
{
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr *my_error_ptr;

METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
    struct my_error_mgr* my_err = reinterpret_cast<struct my_error_mgr*>(cinfo->err);
    longjmp(my_err->setjmp_buffer, 1);
}

extern "C" int gtatool_to_jpeg(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::val<int> quality("quality", 'q', opt::optional, 1, 100, 75);
    options.push_back(&quality);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, 2, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_to_jpeg_help();
        return 0;
    }

    try
    {
        std::string nameo = arguments.size() == 1 ? arguments[0] : arguments[1];
        array_loop_t array_loop;
        gta::header hdr;
        std::string name;

        struct jpeg_compress_struct cinfo;
        struct my_error_mgr jerr;
        JSAMPROW jrow[1];

        array_loop.start(arguments.size() == 1 ? std::vector<std::string>() : std::vector<std::string>(1, arguments[0]), nameo);
        if (array_loop.read(hdr, name))
        {
            if (hdr.dimensions() != 2)
                throw exc(name + ": only two-dimensional arrays can be converted to JPEG.");
            if (hdr.components() != 1 && hdr.components() != 3)
                throw exc(name + ": only arrays with 1 or 3 element components can be converted to JPEG.");
            for (uintmax_t i = 0; i < hdr.components(); i++)
                if (hdr.component_type(i) != gta::uint8)
                    throw exc(name + ": only arrays with element component type uint8 can be converted to JPEG.");

            FILE* jpegfile = fio::open(nameo, "w");

            cinfo.err = jpeg_std_error(&jerr.pub);
            jerr.pub.error_exit = my_error_exit;
            if (setjmp(jerr.setjmp_buffer))
            {
                /* JPEG has signaled an error */
                char message[JMSG_LENGTH_MAX];
                (cinfo.err->format_message)(reinterpret_cast<jpeg_common_struct*>(&cinfo), message);
                jpeg_destroy_compress(&cinfo);
                fio::close(jpegfile);
                throw exc(nameo + ": " + message);
            }

            jpeg_create_compress(&cinfo);
            jpeg_stdio_dest(&cinfo, jpegfile);
            cinfo.image_width = checked_cast<JDIMENSION>(hdr.dimension_size(0));
            cinfo.image_height = checked_cast<JDIMENSION>(hdr.dimension_size(1));
            cinfo.input_components = hdr.components();
            cinfo.in_color_space = (hdr.components() == 1 ? JCS_GRAYSCALE : JCS_RGB);
            jpeg_set_defaults(&cinfo);
            jpeg_set_quality(&cinfo, quality.value(), TRUE);

            jpeg_start_compress(&cinfo, TRUE);

            element_loop_t element_loop;
            array_loop.start_element_loop(element_loop, hdr, gta::header());

            blob row(cinfo.image_width, cinfo.input_components);
            while (cinfo.next_scanline < cinfo.image_height)
            {
                jrow[0] = reinterpret_cast<JSAMPROW>(const_cast<void*>(element_loop.read(hdr.dimension_size(0))));
                jpeg_write_scanlines(&cinfo, jrow, 1);
            }

            jpeg_finish_compress(&cinfo);
            fio::close(jpegfile, nameo);
            jpeg_destroy_compress(&cinfo);
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
