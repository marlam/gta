/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2011, 2012, 2013
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

#include "base/msg.h"
#include "base/blb.h"
#include "base/fio.h"
#include "base/opt.h"
#include "base/str.h"
#include "base/chk.h"

#include "lib.h"

extern "C" void gtatool_from_ply_help(void)
{
    msg::req_txt("from-ply <input-file> [<output-file>]\n"
            "\n"
            "Converts PLY files to GTAs. Currently only point clouds (vertex lists) are supported, "
            "but no faces, edges, or materials. All vertex attributes will be exported.");
}

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
                int num_elems = 0;
                int nprops = 0;
                PlyProperty** plyprop = ply_get_element_description(
                        ply, const_cast<char *>("vertex"), &num_elems, &nprops);

                gta::header hdr;
                std::string nameo;
                hdr.set_dimensions(num_elems);
                std::vector<gta::type> types;
                std::vector<std::string> typetags;
                int type_offset = 0;
                for (int i = 0; i < nprops; i++)
                {
                    size_t type_size = 0;
                    if (plyprop[i]->external_type == PLY_CHAR)
                    {
                        if (std::numeric_limits<char>::min() < 0)
                        {
                            types.push_back(gta::int8);
                            type_size = sizeof(int8_t);
                        }
                        else
                        {
                            types.push_back(gta::uint8);
                            type_size = sizeof(uint8_t);
                        }
                    }
                    else if (plyprop[i]->external_type == PLY_UCHAR || plyprop[i]->external_type == PLY_UINT8)
                    {
                        types.push_back(gta::uint8);
                        type_size = sizeof(uint8_t);
                    }
                    else if (plyprop[i]->external_type == PLY_FLOAT || plyprop[i]->external_type == PLY_FLOAT32)
                    {
                        types.push_back(gta::float32);
                        type_size = sizeof(float);
                    }
                    else if (plyprop[i]->external_type == PLY_DOUBLE)
                    {
                        types.push_back(gta::float64);
                        type_size = sizeof(double);
                    }
                    else if (plyprop[i]->external_type == PLY_SHORT)
                    {
                        types.push_back(gta::int16);
                        type_size = sizeof(int16_t);
                    }
                    else if (plyprop[i]->external_type == PLY_USHORT)
                    {
                        types.push_back(gta::uint16);
                        type_size = sizeof(uint16_t);
                    }
                    else if (plyprop[i]->external_type == PLY_INT)
                    {
                        types.push_back(gta::int32);
                        type_size = sizeof(int32_t);
                    }
                    else if (plyprop[i]->external_type == PLY_UINT)
                    {
                        types.push_back(gta::uint32);
                        type_size = sizeof(uint32_t);
                    }
                    else
                    {
                        throw exc(namei + ": unsupported property type");
                    }
                    std::string propname = plyprop[i]->name;
                    if (propname == "x")
                        typetags.push_back("X");
                    else if (propname == "y")
                        typetags.push_back("Y");
                    else if (propname == "z")
                        typetags.push_back("Z");
                    else if (propname == "nx" || propname == "normal_x")
                        typetags.push_back("X-NORMAL-X");
                    else if (propname == "ny" || propname == "normal_y")
                        typetags.push_back("X-NORMAL-Y");
                    else if (propname == "nz" || propname == "normal_z")
                        typetags.push_back("X-NORMAL-Z");
                    else if (propname == "r" || propname == "red")
                        typetags.push_back("RED");
                    else if (propname == "g" || propname == "green")
                        typetags.push_back("GREEN");
                    else if (propname == "b" || propname == "blue")
                        typetags.push_back("BLUE");
                    else if (propname == "a" || propname == "alpha")
                        typetags.push_back("ALPHA");
                    else
                        typetags.push_back(std::string("X-") + propname);
                    plyprop[i]->internal_type = plyprop[i]->external_type;
                    plyprop[i]->offset = type_offset;
                    plyprop[i]->is_list = 0;
                    plyprop[i]->count_external = 0;
                    plyprop[i]->count_internal = 0;
                    plyprop[i]->count_offset = 0;
                    ply_get_property(ply, "vertex", plyprop[i]);
                    type_offset += type_size;
                }
                hdr.set_components(types.size(), &(types[0]));
                for (size_t i = 0; i < types.size(); i++)
                    hdr.component_taglist(i).set("INTERPRETATION", typetags[i].c_str());
                array_loop.write(hdr, nameo);
                blob element(hdr.element_size());
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, gta::header(), hdr);
                for (uintmax_t e = 0; e < hdr.elements(); e++)
                {
                    ply_get_element(ply, element.ptr());
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
