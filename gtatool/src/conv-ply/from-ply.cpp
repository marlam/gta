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
#include "fio.h"
#include "opt.h"
#include "str.h"
#include "intcheck.h"

#include "lib.h"

extern "C" void gtatool_from_ply_help(void)
{
    msg::req_txt("from-ply <input-file> [<output-file>]\n"
            "\n"
            "Converts PLY files to GTAs. Currently only point clouds (vertex lists) are supported, "
            "but no faces, edges, or materials.\n"
            "Supported vertex properties are x,y,z and r,g,b. Only the x property is required.");
}

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

extern "C" int gtatool_from_ply(int argc, char *argv[])
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
        gtatool_from_ply_help();
        return 0;
    }

    try
    {
        array_loop_t array_loop;
        array_loop.start(std::vector<std::string>(1, arguments[0]), arguments.size() == 2 ? arguments[1] : "");

        std::string namei = arguments[0];
        FILE *fi = fio::open(namei, "r");

        int nelems;
        char **elist;

        PlyFile *ply = ply_read(fi, &nelems, &elist);
        if (!ply)
        {
            throw exc(namei + ": cannot read file.");
        }

        for (int i = 0; i < nelems; i++)
        {
            if (std::strcmp(elist[i], "vertex") == 0)
            {
                int num_elems;
                int nprops;
                (void)ply_get_element_description(ply, const_cast<char *>("vertex"), &num_elems, &nprops);

                gta::header hdr;
                std::string nameo;
                hdr.set_dimensions(num_elems);
                if (nprops == 1)
                {
                    hdr.set_components(gta::float32);
                    hdr.component_taglist(0).set("INTERPRETATION", "X");
                }
                else if (nprops == 2)
                {
                    hdr.set_components(gta::float32, gta::float32);
                    hdr.component_taglist(0).set("INTERPRETATION", "X");
                    hdr.component_taglist(1).set("INTERPRETATION", "Y");
                }
                else if (nprops == 3)
                {
                    hdr.set_components(gta::float32, gta::float32, gta::float32);
                    hdr.component_taglist(0).set("INTERPRETATION", "X");
                    hdr.component_taglist(1).set("INTERPRETATION", "Y");
                    hdr.component_taglist(2).set("INTERPRETATION", "Z");
                }
                else if (nprops == 4)
                {
                    std::vector<gta::type> types(1, gta::float32);
                    types.resize(4, gta::uint8);
                    hdr.set_components(4, &(types[0]));
                    hdr.component_taglist(0).set("INTERPRETATION", "X");
                    hdr.component_taglist(1).set("INTERPRETATION", "RED");
                    hdr.component_taglist(2).set("INTERPRETATION", "GREEN");
                    hdr.component_taglist(3).set("INTERPRETATION", "BLUE");
                }
                else if (nprops == 5)
                {
                    std::vector<gta::type> types(2, gta::float32);
                    types.resize(5, gta::uint8);
                    hdr.set_components(5, &(types[0]));
                    hdr.component_taglist(0).set("INTERPRETATION", "X");
                    hdr.component_taglist(1).set("INTERPRETATION", "Y");
                    hdr.component_taglist(2).set("INTERPRETATION", "RED");
                    hdr.component_taglist(3).set("INTERPRETATION", "GREEN");
                    hdr.component_taglist(4).set("INTERPRETATION", "BLUE");
                }
                else if (nprops == 6)
                {
                    std::vector<gta::type> types(3, gta::float32);
                    types.resize(6, gta::uint8);
                    hdr.set_components(6, &(types[0]));
                    hdr.component_taglist(0).set("INTERPRETATION", "X");
                    hdr.component_taglist(1).set("INTERPRETATION", "Y");
                    hdr.component_taglist(2).set("INTERPRETATION", "Z");
                    hdr.component_taglist(3).set("INTERPRETATION", "RED");
                    hdr.component_taglist(4).set("INTERPRETATION", "GREEN");
                    hdr.component_taglist(5).set("INTERPRETATION", "BLUE");
                }
                else
                {
                    throw exc(namei + ": unsupported number of vertex properties.");
                }
                array_loop.write(hdr, nameo);
                for (int j = 0; j < nprops; j++)
                {
                    ply_get_property(ply, "vertex", &ply_vert_props[j]);
                }
                ply_vertex vertex;
                blob element(hdr.element_size());
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, gta::header(), hdr);
                for (uintmax_t e = 0; e < hdr.elements(); e++)
                {
                    ply_get_element(ply, &vertex);
                    if (nprops == 1)
                    {
                        std::memcpy(hdr.component(element.ptr(), 0), &vertex.x, sizeof(float));
                    }
                    else if (nprops == 2)
                    {
                        std::memcpy(hdr.component(element.ptr(), 0), &vertex.x, sizeof(float));
                        std::memcpy(hdr.component(element.ptr(), 1), &vertex.y, sizeof(float));
                    }
                    else if (nprops == 3)
                    {
                        std::memcpy(hdr.component(element.ptr(), 0), &vertex.x, sizeof(float));
                        std::memcpy(hdr.component(element.ptr(), 1), &vertex.y, sizeof(float));
                        std::memcpy(hdr.component(element.ptr(), 2), &vertex.z, sizeof(float));
                    }
                    else if (nprops == 4)
                    {
                        std::memcpy(hdr.component(element.ptr(), 0), &vertex.x, sizeof(float));
                        std::memcpy(hdr.component(element.ptr(), 1), &vertex.r, sizeof(uint8_t));
                        std::memcpy(hdr.component(element.ptr(), 2), &vertex.g, sizeof(uint8_t));
                        std::memcpy(hdr.component(element.ptr(), 3), &vertex.b, sizeof(uint8_t));
                    }
                    else if (nprops == 5)
                    {
                        std::memcpy(hdr.component(element.ptr(), 0), &vertex.x, sizeof(float));
                        std::memcpy(hdr.component(element.ptr(), 1), &vertex.y, sizeof(float));
                        std::memcpy(hdr.component(element.ptr(), 2), &vertex.r, sizeof(uint8_t));
                        std::memcpy(hdr.component(element.ptr(), 3), &vertex.g, sizeof(uint8_t));
                        std::memcpy(hdr.component(element.ptr(), 4), &vertex.b, sizeof(uint8_t));
                    }
                    else
                    {
                        std::memcpy(hdr.component(element.ptr(), 0), &vertex.x, sizeof(float));
                        std::memcpy(hdr.component(element.ptr(), 1), &vertex.y, sizeof(float));
                        std::memcpy(hdr.component(element.ptr(), 2), &vertex.z, sizeof(float));
                        std::memcpy(hdr.component(element.ptr(), 3), &vertex.r, sizeof(uint8_t));
                        std::memcpy(hdr.component(element.ptr(), 4), &vertex.g, sizeof(uint8_t));
                        std::memcpy(hdr.component(element.ptr(), 5), &vertex.b, sizeof(uint8_t));
                    }
                    element_loop.write(element.ptr());
                }
                break;
            }
            else
            {
                continue;
            }
        }
        fio::close(fi, namei);
        array_loop.finish();
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
