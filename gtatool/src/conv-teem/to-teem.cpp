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
#include <cmath>

#include <gta/gta.hpp>

#include <teem/nrrd.h>

#include "base/msg.h"
#include "base/blb.h"
#include "base/opt.h"
#include "base/str.h"
#include "base/chk.h"

#include "lib.h"

// TODO
// - Set a meaningful type for multi-component elements that are converted to
//   pseudo-axes for NRRD.
// - Handle much more kinds of metatata.

extern "C" void gtatool_to_teem_help(void)
{
    msg::req_txt("to-teem [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to the nrrd format via libteem.");
}

extern "C" int gtatool_to_teem(int argc, char *argv[])
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
        gtatool_to_teem_help();
        return 0;
    }

    try
    {
        std::string nameo = arguments.size() == 1 ? arguments[0] : arguments[1];
        array_loop_t array_loop;
        gta::header hdr;
        std::string name;

        Nrrd *nrrdo;

        array_loop.start(arguments.size() == 1 ? std::vector<std::string>() : std::vector<std::string>(1, arguments[0]), nameo);
        while (array_loop.read(hdr, name))
        {
            if (hdr.data_size() == 0)
                continue;

            gta::type type = hdr.component_type(0);
            uintmax_t type_size = hdr.component_size(0);
            bool type_is_complex = (type == gta::cfloat32 || type == gta::cfloat64);
            for (uintmax_t i = 1; i < hdr.components(); i++)
            {
                if (hdr.component_type(i) != type || hdr.component_size(i) != type_size)
                    throw exc(name + ": element components have different types.");
            }

            std::vector<size_t> dimensions;
            if (hdr.components() > 1 || type_is_complex)
            {
                uintmax_t factor = (type_is_complex ? 2 : 1);
                dimensions.push_back(checked_cast<size_t>(checked_mul(hdr.components(), factor)));
                if (type_is_complex)
                    type = (type == gta::cfloat32 ? gta::float32 : gta::float64);
            }
            for (uintmax_t i = 0; i < hdr.dimensions(); i++)
            {
                dimensions.push_back(checked_cast<size_t>(hdr.dimension_size(i)));
            }

            blob data(checked_cast<size_t>(hdr.data_size()));
            array_loop.read_data(hdr, data.ptr());

            int nrrdo_type;
            switch (type)
            {
            case gta::int8:
                nrrdo_type = nrrdTypeChar;
                break;
            case gta::uint8:
                nrrdo_type = nrrdTypeUChar;
                break;
            case gta::int16:
                nrrdo_type = nrrdTypeShort;
                break;
            case gta::uint16:
                nrrdo_type = nrrdTypeUShort;
                break;
            case gta::int32:
                nrrdo_type = nrrdTypeInt;
                break;
            case gta::uint32:
                nrrdo_type = nrrdTypeUInt;
                break;
            case gta::int64:
                nrrdo_type = nrrdTypeLLong;
                break;
            case gta::uint64:
                nrrdo_type = nrrdTypeULLong;
                break;
            case gta::float32:
                nrrdo_type = nrrdTypeFloat;
                break;
            case gta::float64:
                nrrdo_type = nrrdTypeDouble;
                break;
            case gta::blob:
                nrrdo_type = nrrdTypeBlock;
                break;
            default:
                throw exc(name + ": component type not supported.");
            }

            nrrdo = nrrdNew();
            if (nrrdWrap_nva(nrrdo, data.ptr(), nrrdo_type,
                        checked_cast<unsigned int>(dimensions.size()), &(dimensions[0])))
            {
                char* errptr = biffGetDone(NRRD);
                std::string errstr(errptr);
                std::free(errptr);
                throw exc(name + ": " + errstr);
            }
            if (type == gta::blob)
            {
                nrrdo->blockSize = checked_cast<size_t>(type_size);
            }
            if (nrrdSave(nameo.c_str(), nrrdo, NULL))
            {
                char* errptr = biffGetDone(NRRD);
                std::string errstr(errptr);
                std::free(errptr);
                throw exc(name + ": " + errstr);
            }
            nrrdNix(nrrdo);
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
