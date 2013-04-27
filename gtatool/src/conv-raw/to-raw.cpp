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
#include <limits>

#include <gta/gta.hpp>

#include "base/msg.h"
#include "base/blb.h"
#include "base/fio.h"
#include "base/opt.h"
#include "base/chk.h"
#include "base/end.h"

#include "lib.h"


extern "C" void gtatool_to_raw_help(void)
{
    msg::req_txt("to-raw [-e|--endianness=little|big] [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to raw binary format. The default endianness is little.\n"
            "Example: to-raw data.gta data.raw");
}

extern "C" int gtatool_to_raw(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> endiannesses;
    endiannesses.push_back("little");
    endiannesses.push_back("big");
    opt::string endian("endianness", 'e', opt::optional, endiannesses, "little");
    options.push_back(&endian);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, 2, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_to_raw_help();
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
        std::string namei;
        std::string nameo = arguments.size() == 1 ? arguments[0] : arguments[1];
        gta::header hdri;
        gta::header hdro;
        array_loop_t array_loop;

        array_loop.start(arguments.size() == 1 ? std::vector<std::string>() : std::vector<std::string>(1, arguments[0]), nameo);
        while (array_loop.read(hdri, namei))
        {
            hdro = hdri;
            hdro.set_compression(gta::none);
            if (host_endianness)
            {
                array_loop.copy_data(hdri, hdro);
            }
            else
            {
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, hdri, hdro);
                blob element(checked_cast<size_t>(hdri.element_size()));
                for (uintmax_t e = 0; e < hdri.elements(); e++)
                {
                    std::memcpy(element.ptr(), element_loop.read(1), hdri.element_size());
                    swap_element_endianness(hdri, element.ptr());
                    element_loop.write(element.ptr());
                }
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
