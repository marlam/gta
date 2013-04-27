/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010, 2011, 2012, 2013
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

#include "base/msg.h"
#include "base/blb.h"
#include "base/fio.h"
#include "base/opt.h"
#include "base/chk.h"
#include "base/end.h"

#include "lib.h"


extern "C" void gtatool_from_raw_help(void)
{
    msg::req_txt("from-raw -d|--dimensions=<d0,d1,...> -c|--components=<c0,c1,...>\n"
            "    [-e|--endianness=little|big] <input-file> [<output-file>]\n"
            "\n"
            "Converts raw binary files to GTAs. The default endianness is little.\n"
            "Available component types: int8, uint8, int16, uint16, int32, uint32, "
            "int64, uint64, int128, uint128, float32, float64, float128, cfloat32, "
            "cfloat64, cfloat128.\n"
            "Example: from-raw -d 640,480 -c uint8,uint8,uint8 -e little file.raw");
}

extern "C" int gtatool_from_raw(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::tuple<uintmax_t> dimensions("dimensions", 'd', opt::required, 1, std::numeric_limits<uintmax_t>::max());
    options.push_back(&dimensions);
    opt::string components("components", 'c', opt::required);
    options.push_back(&components);
    std::vector<std::string> endiannesses;
    endiannesses.push_back("little");
    endiannesses.push_back("big");
    opt::val<std::string> endian("endianness", 'e', opt::optional, endiannesses, "little");
    options.push_back(&endian);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, 2, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_from_raw_help();
        return 0;
    }

    bool host_endianness;
    if (endianness::endianness == endianness::big)
    {
        host_endianness = (endian.value().compare("big") == 0);
    }
    else
    {
        host_endianness = (endian.value().compare("little") == 0);
    }

    try
    {
        gta::header hdr;
        hdr.set_dimensions(dimensions.value().size(), &(dimensions.value()[0]));
        std::vector<gta::type> comp_types;
        std::vector<uintmax_t> comp_sizes;
        typelist_from_string(components.value(), &comp_types, &comp_sizes);
        hdr.set_components(comp_types.size(), &(comp_types[0]), comp_sizes.size() == 0 ? NULL : &(comp_sizes[0]));

        array_loop_t array_loop;
        array_loop.start(std::vector<std::string>(1, arguments[0]), arguments.size() == 2 ? arguments[1] : "");
        std::string nameo;

        do
        {
            array_loop.write(hdr, nameo);

            if (host_endianness)
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
        }
        while (fio::has_more(array_loop.file_in()));
        array_loop.finish();
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
