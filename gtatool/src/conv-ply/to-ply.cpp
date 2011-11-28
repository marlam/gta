/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2011
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
#include "cio.h"
#include "opt.h"
#include "str.h"
#include "intcheck.h"

#include "lib.h"

extern "C" void gtatool_to_ply_help(void)
{
    msg::req_txt("to-ply [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to the PLY format. Currently only point clouds (vertex lists) are supported, "
            "but no faces, edges, or materials. The input GTA must be one-dimensional. If you want to "
            "generate dense point clouds e.g. from an image or a volume, use the dimension-flatten command before.\n"
            "Supported vertex properties are x,y,z and r,g,b. Only the x property is required. Coordinates must "
            "have type float32 and color components must have type uint8. Use the component-convert command if "
            "necessary.");
}

static const char *ply_elem_names[] = { "vertex" };

typedef struct
{
    float x, y, z;
    uint8_t r, g, b;
} ply_vertex;

static PlyProperty ply_vert_props[] =
{
    { "x", PLY_FLOAT, PLY_FLOAT, offsetof(ply_vertex, x), 0, 0, 0, 0 },
    { "y", PLY_FLOAT, PLY_FLOAT, offsetof(ply_vertex, y), 0, 0, 0, 0 },
    { "z", PLY_FLOAT, PLY_FLOAT, offsetof(ply_vertex, z), 0, 0, 0, 0 },
    { "r", PLY_UCHAR, PLY_UCHAR, offsetof(ply_vertex, r), 0, 0, 0, 0 },
    { "g", PLY_UCHAR, PLY_UCHAR, offsetof(ply_vertex, g), 0, 0, 0, 0 },
    { "b", PLY_UCHAR, PLY_UCHAR, offsetof(ply_vertex, b), 0, 0, 0, 0 },
};

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
            if (hdr.dimensions() != 1)
            {
                throw exc(name + ": only one-dimensional arrays can be converted to PLY.");
            }
            if (hdr.components() > 6)
            {
                throw exc(name + ": unsupported number of element components.");
            }
            if (hdr.components() >= 4)
            {
                for (uintmax_t c = hdr.components() - 3; c < hdr.components(); c++)
                {
                    if (hdr.component_type(c) != gta::uint8)
                    {
                        throw exc(name + ": color components must have type uint8.");
                    }
                }
            }
            for (uintmax_t c = 0; c < (hdr.components() < 4 ? hdr.components() : hdr.components() - 3); c++)
            {
                if (hdr.component_type(c) != gta::float32)
                {
                    throw exc(name + ": coordinates must have type float32.");
                }
            }

            FILE *fo = cio::open(nameo, "w");
            PlyFile *ply = ply_write(fo, 1, ply_elem_names, PLY_BINARY_LE);
            if (!ply)
            {
                throw exc(nameo + ": cannot write file.");
            }

            ply_element_count(ply, "vertex", checked_cast<int>(hdr.elements()));

            if (hdr.components() == 1)
            {
                ply_describe_property(ply, "vertex", &ply_vert_props[0]);
            }
            else if (hdr.components() == 2)
            {
                ply_describe_property(ply, "vertex", &ply_vert_props[0]);
                ply_describe_property(ply, "vertex", &ply_vert_props[1]);
            }
            else if (hdr.components() == 3)
            {
                ply_describe_property(ply, "vertex", &ply_vert_props[0]);
                ply_describe_property(ply, "vertex", &ply_vert_props[1]);
                ply_describe_property(ply, "vertex", &ply_vert_props[2]);
            }
            else if (hdr.components() == 4)
            {
                ply_describe_property(ply, "vertex", &ply_vert_props[0]);
                ply_describe_property(ply, "vertex", &ply_vert_props[3]);
                ply_describe_property(ply, "vertex", &ply_vert_props[4]);
                ply_describe_property(ply, "vertex", &ply_vert_props[5]);
            }
            else if (hdr.components() == 5)
            {
                ply_describe_property(ply, "vertex", &ply_vert_props[0]);
                ply_describe_property(ply, "vertex", &ply_vert_props[1]);
                ply_describe_property(ply, "vertex", &ply_vert_props[3]);
                ply_describe_property(ply, "vertex", &ply_vert_props[4]);
                ply_describe_property(ply, "vertex", &ply_vert_props[5]);
            }
            else
            {
                ply_describe_property(ply, "vertex", &ply_vert_props[0]);
                ply_describe_property(ply, "vertex", &ply_vert_props[1]);
                ply_describe_property(ply, "vertex", &ply_vert_props[2]);
                ply_describe_property(ply, "vertex", &ply_vert_props[3]);
                ply_describe_property(ply, "vertex", &ply_vert_props[4]);
                ply_describe_property(ply, "vertex", &ply_vert_props[5]);
            }
            ply_header_complete(ply);

            element_loop_t element_loop;
            array_loop.start_element_loop(element_loop, hdr, gta::header());
            ply_put_element_setup(ply, "vertex");
            ply_vertex v;
            for (uintmax_t e = 0; e < hdr.elements(); e++)
            {
                void *p = element_loop.read();
                if (hdr.components() == 1)
                {
                    std::memcpy(&v.x, hdr.component(p, 0), sizeof(float));
                }
                else if (hdr.components() == 2)
                {
                    std::memcpy(&v.x, hdr.component(p, 0), sizeof(float));
                    std::memcpy(&v.y, hdr.component(p, 1), sizeof(float));
                }
                else if (hdr.components() == 3)
                {
                    std::memcpy(&v.x, hdr.component(p, 0), sizeof(float));
                    std::memcpy(&v.y, hdr.component(p, 1), sizeof(float));
                    std::memcpy(&v.z, hdr.component(p, 2), sizeof(float));
                }
                else if (hdr.components() == 4)
                {
                    std::memcpy(&v.x, hdr.component(p, 0), sizeof(float));
                    std::memcpy(&v.r, hdr.component(p, 1), sizeof(uint8_t));
                    std::memcpy(&v.g, hdr.component(p, 2), sizeof(uint8_t));
                    std::memcpy(&v.b, hdr.component(p, 3), sizeof(uint8_t));
                }
                else if (hdr.components() == 5)
                {
                    std::memcpy(&v.x, hdr.component(p, 0), sizeof(float));
                    std::memcpy(&v.y, hdr.component(p, 1), sizeof(float));
                    std::memcpy(&v.r, hdr.component(p, 2), sizeof(uint8_t));
                    std::memcpy(&v.g, hdr.component(p, 3), sizeof(uint8_t));
                    std::memcpy(&v.b, hdr.component(p, 4), sizeof(uint8_t));
                }
                else
                {
                    std::memcpy(&v.x, hdr.component(p, 0), sizeof(float));
                    std::memcpy(&v.y, hdr.component(p, 1), sizeof(float));
                    std::memcpy(&v.z, hdr.component(p, 2), sizeof(float));
                    std::memcpy(&v.r, hdr.component(p, 3), sizeof(uint8_t));
                    std::memcpy(&v.g, hdr.component(p, 4), sizeof(uint8_t));
                    std::memcpy(&v.b, hdr.component(p, 5), sizeof(uint8_t));
                }
                ply_put_element(ply, &v);
            }
            cio::flush(fo, nameo);
            if (std::ferror(fo))
            {
                throw exc(nameo + ": output error.");
            }
            // ply_close(ply); // This crashes for some reason.
            // So we just close() the file and accept the memory leakage.
            cio::close(fo, nameo);
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
