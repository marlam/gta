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

#include <gta/gta.hpp>

#include "msg.h"
#include "str.h"
#include "blob.h"
#include "fio.h"
#include "opt.h"
#include "intcheck.h"
#include "endianness.h"

#include "lib.h"


extern "C" void gtatool_from_datraw_help(void)
{
    msg::req_txt("from-datraw <input-file> [<output-file>]\n"
            "\n"
            "Converts .dat/.raw volumetric data files to GTAs.\n"
            "The input file should be the .dat file; this command will open the .raw file automatically.");
}

extern "C" int gtatool_from_datraw(int argc, char *argv[])
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
        gtatool_from_datraw_help();
        return 0;
    }

    try
    {
        std::string namei = arguments[0];
        std::string dat_objectfilename;
        unsigned int dat_res_x = 0, dat_res_y = 0, dat_res_z = 0;
        float dat_slicethickness_x = 1.0f, dat_slicethickness_y = 1.0f, dat_slicethickness_z = 1.0f;
        gta::type dat_type = gta::blob;
        bool dat_hostendianness = true;

        FILE* fdat = fio::open(namei, "r");
        while (!::feof(fdat))
        {
            std::string line = str::trim(fio::readline(fdat, namei));
            if (line.length() == 0 || line[0] == '#')
                continue;
            if (line.substr(0, 11) == "Resolution:")
            {
                std::vector<std::string> tokens = str::tokens(line.substr(11), " \t");
                if (tokens.size() != 3)
                    throw exc(namei + ": invalid Resolution field");
                dat_res_x = str::to<unsigned int>(tokens[0]);
                dat_res_y = str::to<unsigned int>(tokens[1]);
                dat_res_z = str::to<unsigned int>(tokens[2]);
                if (dat_res_x < 1 || dat_res_y < 1 || dat_res_z < 1)
                    throw exc(namei + ": invalid Resolution field");
            }
            else if (line.substr(0, 15) == "ObjectFileName:")
            {
                dat_objectfilename = fio::from_sys(str::trim(line.substr(15)));
                if (dat_objectfilename.length() > 0 && dat_objectfilename[0] != '/')
                {
                    // relative file name; prepend the directory of the dat file
                    dat_objectfilename = fio::dirname(namei) + '/' + dat_objectfilename;
                }
            }
            else if (line.substr(0, 15) == "SliceThickness:")
            {
                std::vector<std::string> tokens = str::tokens(line.substr(15), " \t");
                if (tokens.size() != 3)
                    throw exc(namei + ": invalid SliceThickness field");
                dat_slicethickness_x = str::to<float>(tokens[0]);
                dat_slicethickness_y = str::to<float>(tokens[1]);
                dat_slicethickness_z = str::to<float>(tokens[2]);
                if (dat_slicethickness_x <= 0.0f || dat_slicethickness_y <= 0.0f || dat_slicethickness_z <= 0.0f)
                    throw exc(namei + ": invalid SliceThickness field");
            }
            else if (line.substr(0, 7) == "Format:")
            {
                std::string token = str::trim(line.substr(7));
                if (token == "UCHAR")
                    dat_type = gta::uint8;
                else if (token == "USHORT")
                    dat_type = gta::uint16;
                else if (token == "FLOAT")
                    dat_type = gta::float32;
                else
                    throw exc(namei + ": invalid Format field");
            }
            else if (line.substr(0, 10) == "ByteOrder:")
            {
                std::string s = str::trim(line.substr(10));
                if (s == "big-endian" || s == "bigendian" || s == "bigEndian")
                    dat_hostendianness = (endianness::endianness == endianness::big);
                else if (s == "little-endian" || s == "littleendian" || s == "littleEndian")
                    dat_hostendianness = (endianness::endianness == endianness::little);
                else
                    throw exc(namei + ": invalid ByteOrder field");
            }
        }
        if (dat_objectfilename.length() == 0)
            throw exc(namei + ": missing ObjectFileName field");
        if (dat_res_x < 1 || dat_res_y < 1 || dat_res_z < 1)
            throw exc(namei + ": missing Resolution field");
        if (dat_type == gta::blob)
            throw exc(namei + ": missing Format field");

        gta::header hdr;
        hdr.set_components(dat_type);
        hdr.set_dimensions(dat_res_x, dat_res_y, dat_res_z);
        if (dat_slicethickness_x < 1.0f || dat_slicethickness_x > 1.0f)
            hdr.dimension_taglist(0).set("SAMPLE-DISTANCE", str::from(dat_slicethickness_x).c_str());
        if (dat_slicethickness_y < 1.0f || dat_slicethickness_y > 1.0f)
            hdr.dimension_taglist(1).set("SAMPLE-DISTANCE", str::from(dat_slicethickness_y).c_str());
        if (dat_slicethickness_z < 1.0f || dat_slicethickness_z > 1.0f)
            hdr.dimension_taglist(2).set("SAMPLE-DISTANCE", str::from(dat_slicethickness_z).c_str());

        array_loop_t array_loop;
        array_loop.start(std::vector<std::string>(1, dat_objectfilename), arguments.size() == 2 ? arguments[1] : "");
        std::string nameo;
        array_loop.write(hdr, nameo);

        if (dat_hostendianness)
        {
            array_loop.copy_data(hdr, hdr);
        }
        else
        {
            element_loop_t element_loop;
            array_loop.start_element_loop(element_loop, hdr, hdr);
            blob element(checked_cast<size_t>(hdr.element_size()));
            for (uintmax_t e = 0; e < hdr.elements(); e++)
            {
                std::memcpy(element.ptr(), element_loop.read(1), hdr.element_size());
                swap_element_endianness(hdr, element.ptr());
                element_loop.write(element.ptr());
            }
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
