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
#include <limits>
#include <cstdlib>

#include <gta/gta.hpp>

#include <teem/nrrd.h>

#include "msg.h"
#include "blob.h"
#include "opt.h"
#include "str.h"
#include "intcheck.h"

#include "lib.h"

// TODO
// - Transfrom pseudo-axes that represent non-scalar data into
//   multi-component elements!
// - Handle key/value pair metadata. Which format do these have in NRRD?
// - Handle other kinds of metatata.

extern "C" void gtatool_from_teem_help(void)
{
    msg::req_txt("from-teem <input-file> [<output-file>]\n"
            "\n"
            "Converts nnrd files to GTAs.");
}

extern "C" int gtatool_from_teem(int argc, char *argv[])
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
        gtatool_from_teem_help();
        return 0;
    }

    try
    {
        array_loop_t array_loop;
        array_loop.start(std::vector<std::string>(1, arguments[0]), arguments.size() == 2 ? arguments[1] : "");

        std::string namei = arguments[0];
        Nrrd *nrrdi;

        gta::header hdr;
        std::string nameo;

        nrrdi = nrrdNew();
        if (nrrdLoad(nrrdi, namei.c_str(), NULL))
        {
            char* errptr = biffGetDone(NRRD);
            std::string errstr(errptr);
            std::free(errptr);
            throw exc(namei + ": " + errstr);
        }

        if (nrrdi->content)
            hdr.global_taglist().set("DESCRIPTION", nrrdi->content);
        for (unsigned int i = 0; i < nrrdi->cmtArr->len; i++)
            hdr.global_taglist().set(("X-COMMENT-" + str::from(i)).c_str(), nrrdi->cmt[i]);

        std::vector<uintmax_t> dimensions;
        for (unsigned int i = 0; i < nrrdi->dim; i++)
        {
            dimensions.push_back(nrrdi->axis[i].size);
        }
        hdr.set_dimensions(dimensions.size(), &(dimensions[0]));

        gta::type type;
        uintmax_t blob_size = 0;
        switch (nrrdi->type)
        {
        case nrrdTypeChar:
            type = gta::int8;
            break;
        case nrrdTypeUChar:
            type = gta::uint8;
            break;
        case nrrdTypeShort:
            type = gta::int16;
            break;
        case nrrdTypeUShort:
            type = gta::uint16;
            break;
        case nrrdTypeInt:
            type = gta::int32;
            break;
        case nrrdTypeUInt:
            type = gta::uint32;
            break;
        case nrrdTypeLLong:
            type = gta::int64;
            break;
        case nrrdTypeULLong:
            type = gta::uint64;
            break;
        case nrrdTypeFloat:
            type = gta::float32;
            break;
        case nrrdTypeDouble:
            type = gta::float64;
            break;
        case nrrdTypeBlock:
            type = gta::blob;
            blob_size = nrrdi->blockSize;
            break;
        default:
            throw exc(namei + ": invalid scalar data type");
        }
        hdr.set_components(type, blob_size);
        if (nrrdi->sampleUnits)
            hdr.component_taglist(0).set("UNIT", nrrdi->sampleUnits);

        array_loop.write(hdr, nameo);
        array_loop.write_data(hdr, nrrdi->data);
        array_loop.finish();
        nrrdNuke(nrrdi);
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
