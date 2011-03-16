/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010, 2011
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
#include "blob.h"
#include "cio.h"
#include "opt.h"
#include "debug.h"
#include "intcheck.h"
#include "endianness.h"

#include "lib.h"


class opt_components : public opt::option
{
private:
    std::vector<gta::type> _types;

public:
    opt_components(const std::string &longname, char shortname, opt::option_policy policy)
        : opt::option(longname, shortname, policy)
    {
    }
    ~opt_components()
    {
    }

    enum opt::option::argument_policy argument_policy() const
    {
        return opt::option::required_argument;
    }

    bool parse_argument(const std::string &s)
    {
        size_t i = 0;
        do
        {
            size_t comma = s.find(',', i);
            size_t component_len = (comma != std::string::npos) ? comma - i : s.length() - i;
            std::string component = s.substr(i, component_len);
            if (component.compare("int8") == 0)
            {
                _types.push_back(gta::int8);
            }
            else if (component.compare("uint8") == 0)
            {
                _types.push_back(gta::uint8);
            }
            else if (component.compare("int16") == 0)
            {
                _types.push_back(gta::int16);
            }
            else if (component.compare("uint16") == 0)
            {
                _types.push_back(gta::uint16);
            }
            else if (component.compare("int32") == 0)
            {
                _types.push_back(gta::int32);
            }
            else if (component.compare("uint32") == 0)
            {
                _types.push_back(gta::uint32);
            }
            else if (component.compare("int64") == 0)
            {
                _types.push_back(gta::int64);
            }
            else if (component.compare("uint64") == 0)
            {
                _types.push_back(gta::uint64);
            }
            else if (component.compare("int128") == 0)
            {
                _types.push_back(gta::int128);
            }
            else if (component.compare("uint128") == 0)
            {
                _types.push_back(gta::uint128);
            }
            else if (component.compare("float32") == 0)
            {
                _types.push_back(gta::float32);
            }
            else if (component.compare("float64") == 0)
            {
                _types.push_back(gta::float64);
            }
            else if (component.compare("float128") == 0)
            {
                _types.push_back(gta::float128);
            }
            else if (component.compare("cfloat32") == 0)
            {
                _types.push_back(gta::cfloat32);
            }
            else if (component.compare("cfloat64") == 0)
            {
                _types.push_back(gta::cfloat64);
            }
            else if (component.compare("cfloat128") == 0)
            {
                _types.push_back(gta::cfloat128);
            }
            else
            {
                return false;
            }
            i = (comma != std::string::npos) ? comma + 1 : std::string::npos;
        }
        while (i < std::string::npos);
        return true;
    }

    const std::vector<gta::type> &value() const
    {
        return _types;
    }
};


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
    opt::tuple<uintmax_t> dimensions("dimensions", 'd', opt::required);
    options.push_back(&dimensions);
    opt_components components("components", 'c', opt::required);
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

    for (size_t i = 0; i < dimensions.value().size(); i++)
    {
        if (dimensions.value()[i] < 1)
        {
            msg::err_txt("Dimension sizes must be greater than zero.");
            return 1;
        }
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

    FILE *fo = gtatool_stdout;
    std::string ifilename(arguments[0]);
    std::string ofilename("standard output");
    try
    {
        if (arguments.size() == 2)
        {
            ofilename = arguments[1];
            fo = cio::open(ofilename, "w");
        }
        if (cio::isatty(fo))
        {
            throw exc("refusing to write to a tty");
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    try
    {
        FILE *fi = cio::open(ifilename, "r");
        gta::header hdr;
        hdr.set_dimensions(dimensions.value().size(), &(dimensions.value()[0]));
        hdr.set_components(components.value().size(), &(components.value()[0]));
        hdr.write_to(fo);
        blob element(checked_cast<size_t>(hdr.element_size()));
        gta::io_state so;
        for (uintmax_t e = 0; e < hdr.elements(); e++)
        {
            cio::read(element.ptr(), hdr.element_size(), 1, fi, ifilename);
            if (!host_endianness)
            {
                swap_element_endianness(hdr, element.ptr());
            }
            hdr.write_elements(so, fo, 1, element.ptr());
        }
        cio::close(fi);
        if (fo != gtatool_stdout)
        {
            cio::close(fo);
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
