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
#include <limits>

#include <gta/gta.hpp>

#include "base/msg.h"
#include "base/str.h"
#include "base/blb.h"
#include "base/fio.h"
#include "base/opt.h"
#include "base/chk.h"
#include "base/end.h"

#include "lib.h"


extern "C" void gtatool_to_datraw_help(void)
{
    msg::req_txt("to-datraw [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to  .dat/.raw volumetric data files.\n"
            "The output file should be the .dat file; this command will write the .raw file automatically.");
}

extern "C" int gtatool_to_datraw(int argc, char *argv[])
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
        gtatool_to_datraw_help();
        return 0;
    }

    try
    {
        std::string namei;
        std::string nameo = arguments.size() == 1 ? arguments[0] : arguments[1];
        std::string raw_nameo = nameo;
        if (raw_nameo.length() > 4
                && (raw_nameo.substr(raw_nameo.length() - 4) == ".dat"
                    || raw_nameo.substr(raw_nameo.length() - 4) == ".DAT"))
        {
            raw_nameo = raw_nameo.substr(0, raw_nameo.length() - 4);
        }
        raw_nameo += ".raw";
        gta::header hdri;
        gta::header hdro;
        array_loop_t array_loop;

        array_loop.start(arguments.size() == 1 ? std::vector<std::string>() : std::vector<std::string>(1, arguments[0]), raw_nameo);
        while (array_loop.read(hdri, namei))
        {
            hdro = hdri;
            hdro.set_compression(gta::none);
            if (hdro.dimensions() < 1 || hdro.dimensions() > 3)
                throw exc(namei + ": unsupported number of dimensions");
            if (hdro.components() != 1)
                throw exc(namei + ": unsupported number of element components");
            if (hdro.component_type(0) != gta::uint8
                    && hdro.component_type(0) != gta::uint16
                    && hdro.component_type(0) != gta::float32)
                throw exc(namei + ": unsupported element component type");

            FILE* datf = fio::open(nameo, "w");
            ::fprintf(datf, "ObjectFileName: %s\r\n", fio::basename(raw_nameo.c_str()).c_str());
            ::fprintf(datf, "TaggedFileName: ---\r\n");                         // only for compat with OpenQVis
            ::fprintf(datf, "Resolution: %s %s %s\r\n",
                    str::from(hdro.dimension_size(0)).c_str(),
                    hdro.dimensions() > 1 ? str::from(hdro.dimension_size(1)).c_str() : "1",
                    hdro.dimensions() > 2 ? str::from(hdro.dimension_size(2)).c_str() : "1");
            const char* tagval0; const char* tagval1; const char* tagval2;
            ::fprintf(datf, "SliceThickness: %s %s %s\r\n",
                    (tagval0 = hdro.dimension_taglist(0).get("SAMPLE-DISTANCE")) ? tagval0 : "1",
                    hdro.dimensions() > 1 && (tagval1 = hdro.dimension_taglist(1).get("SAMPLE-DISTANCE")) ? tagval1 : "1",
                    hdro.dimensions() > 2 && (tagval2 = hdro.dimension_taglist(2).get("SAMPLE-DISTANCE")) ? tagval2 : "1");
            ::fprintf(datf, "Format: %s\r\n",
                    hdro.component_type(0) == gta::uint8 ? "UCHAR"
                    : hdro.component_type(0) == gta::uint16 ? "USHORT"
                    : "FLOAT");
            ::fprintf(datf, "NbrTags: 0\r\n");                                  // only for compat with OpenQVis
            ::fprintf(datf, "ObjectType: TEXTURE_VOLUME_OBJECT\r\n");           // only for compat with OpenQVis
            ::fprintf(datf, "ObjectModel: RGBA\r\n");                           // only for compat with OpenQVis
            ::fprintf(datf, "GridType: EQUIDISTANT\r\n");                       // only for compat with OpenQVis
            ::fprintf(datf, "ByteOrder: %s\r\n",
                    endianness::endianness == endianness::big ? "big-endian" : "little-endian");

            array_loop.copy_data(hdri, hdro);
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
