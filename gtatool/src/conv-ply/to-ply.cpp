/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2011, 2012
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

#include <gta/gta.hpp>

#include "ply.h"

#include "msg.h"
#include "blob.h"
#include "fio.h"
#include "opt.h"
#include "str.h"
#include "intcheck.h"
#include "endianness.h"

#include "lib.h"

extern "C" void gtatool_to_ply_help(void)
{
    msg::req_txt("to-ply [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to the PLY format.\n"
            "All array elements are exported into a single vertex list.");
}

extern "C" int gtatool_to_ply(int argc, char *argv[])
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
        gtatool_to_ply_help();
        return 0;
    }

    try
    {
        std::string nameo = arguments.size() == 1 ? arguments[0] : arguments[1];
        array_loop_t array_loop;
        gta::header hdr;
        std::string name;

        array_loop.start(arguments.size() == 1 ? std::vector<std::string>() : std::vector<std::string>(1, arguments[0]), nameo);
        while (array_loop.read(hdr, name))
        {
            if (hdr.elements() == 0)
            {
                msg::wrn(name + ": skipping empty array");
                continue;
            }
            FILE *fo = fio::open(nameo, "w");
            static const char *elem_names[] = { "vertex" };
            PlyFile *ply = ply_write(fo, 1, elem_names,
                    endianness::endianness == endianness::big ? PLY_BINARY_BE : PLY_BINARY_LE);
            if (!ply)
            {
                throw exc(nameo + ": cannot write file.");
            }
            ply_element_count(ply, "vertex", checked_cast<int>(hdr.elements()));
            PlyProperty prop;
            prop.offset = 0;
            prop.is_list = 0;
            prop.count_external = 0;
            prop.count_internal = 0;
            prop.count_offset = 0;
            for (uintmax_t i = 0; i < hdr.components(); i++)
            {
                const char* tmptagval = hdr.component_taglist(i).get("INTERPRETATION");
                std::string tagval;
                if (tmptagval)
                    tagval = tmptagval;
                if (tagval == "X")
                    tagval = "x";
                else if (tagval == "Y")
                    tagval = "y";
                else if (tagval == "Z")
                    tagval = "z";
                else if (tagval == "X-NORMAL-X")
                    tagval = "nx";
                else if (tagval == "X-NORMAL-Y")
                    tagval = "ny";
                else if (tagval == "X-NORMAL-Z")
                    tagval = "nz";
                else if (tagval == "RED" || tagval == "SRGB/RED")
                    tagval = "r";
                else if (tagval == "GREEN" || tagval == "SRGB/GREEN")
                    tagval = "g";
                else if (tagval == "BLUE" || tagval == "SRGB/BLUE")
                    tagval = "b";
                else if (tagval == "ALPHA")
                    tagval = "a";
                else if (!tagval.empty() && tagval.substr(0, 2) == "X-")
                    tagval = tagval.substr(2);
                else if (tagval.empty())
                    tagval = std::string("component-") + str::from(i);
                prop.name = tagval.c_str();
                if (hdr.component_type(i) == gta::int8 && std::numeric_limits<char>::min() < 0)
                    prop.external_type = prop.internal_type = PLY_CHAR;
                else if (hdr.component_type(i) == gta::uint8)
                    prop.external_type = prop.internal_type = PLY_UCHAR;
                else if (hdr.component_type(i) == gta::int16)
                    prop.external_type = prop.internal_type = PLY_SHORT;
                else if (hdr.component_type(i) == gta::uint16)
                    prop.external_type = prop.internal_type = PLY_USHORT;
                else if (hdr.component_type(i) == gta::int32)
                    prop.external_type = prop.internal_type = PLY_INT;
                else if (hdr.component_type(i) == gta::uint32)
                    prop.external_type = prop.internal_type = PLY_UINT;
                else if (hdr.component_type(i) == gta::float32)
                    prop.external_type = prop.internal_type = PLY_FLOAT;
                else if (hdr.component_type(i) == gta::float64)
                    prop.external_type = prop.internal_type = PLY_DOUBLE;
                else
                    throw exc(name + ": unexportable element component type");
                ply_describe_property(ply, "vertex", &prop);
                prop.offset = checked_add(prop.offset, checked_cast<int>(hdr.component_size(i)));
            }
            ply_header_complete(ply);

            element_loop_t element_loop;
            array_loop.start_element_loop(element_loop, hdr, gta::header());
            ply_put_element_setup(ply, "vertex");
            for (uintmax_t e = 0; e < hdr.elements(); e++)
            {
                const void *p = element_loop.read();
                ply_put_element(ply, const_cast<void*>(p));
            }
            fio::flush(fo, nameo);
            if (std::ferror(fo))
            {
                throw exc(nameo + ": output error.");
            }
            // ply_close(ply); // This crashes for some reason.
            // So we just close() the file and accept the memory leakage.
            fio::close(fo, nameo);
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
