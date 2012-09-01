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

#include <gta/gta.hpp>

#include "msg.h"
#include "str.h"
#include "exc.h"
#include "opt.h"
#include "intcheck.h"
#include "blob.h"

#include "lib.h"

#include "ddsbase.h"


extern "C" void gtatool_to_pvm_help(void)
{
    msg::req_txt("to-pvm [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to the pvm file format.");
}

extern "C" int gtatool_to_pvm(int argc, char *argv[])
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
        gtatool_to_pvm_help();
        return 0;
    }

    try
    {
        std::string nameo = arguments.size() == 1 ? arguments[0] : arguments[1];
        array_loop_t array_loop;
        gta::header hdr;
        std::string name;
        const char* tagval;

        array_loop.start(arguments.size() == 1 ? std::vector<std::string>() : std::vector<std::string>(1, arguments[0]), nameo);
        while (array_loop.read(hdr, name))
        {
            if (hdr.data_size() == 0)
            {
                msg::inf(name + ": skipping empty array");
                continue;
            }

            if (hdr.components() != 1)
            {
                throw exc(name + ": more than one element component");
            }
            gta::type type = hdr.component_type(0);
            if (type != gta::uint8 && type != gta::uint16)
            {
                throw exc(name + ": element component type neither uint8 nor uint16");
            }
            if (hdr.dimensions() < 1 || hdr.dimensions() > 3)
            {
                throw exc(name + ": unsupported number of dimensions");
            }

            blob data(checked_cast<size_t>(hdr.data_size()));
            array_loop.read_data(hdr, data.ptr());

            unsigned int pvm_width = checked_cast<unsigned int>(hdr.dimension_size(0));
            unsigned int pvm_height = 1;
            if (hdr.dimensions() > 1)
                pvm_height = checked_cast<unsigned int>(hdr.dimension_size(1));
            unsigned int pvm_depth = 1;
            if (hdr.dimensions() > 2)
                pvm_depth = checked_cast<unsigned int>(hdr.dimension_size(2));
            unsigned int pvm_components = (type == gta::uint8 ? 1 : 2);
            float pvm_scalex = 1.0f;
            if ((tagval = hdr.dimension_taglist(0).get("SAMPLE-DISTANCE")))
                pvm_scalex = str::to<float>(tagval);
            float pvm_scaley = 1.0f;
            if (hdr.dimensions() > 1 && (tagval = hdr.dimension_taglist(1).get("SAMPLE-DISTANCE")))
                pvm_scaley = str::to<float>(tagval);
            float pvm_scalez = 1.0f;
            if (hdr.dimensions() > 2 && (tagval = hdr.dimension_taglist(2).get("SAMPLE-DISTANCE")))
                pvm_scalez = str::to<float>(tagval);
            unsigned char* pvm_description = NULL;
            std::string multiline_description;
            if ((tagval = hdr.global_taglist().get("DESCRIPTION")))
            {
                multiline_description = tagval;
                str::replace(multiline_description, "\\n", "\n");
                pvm_description = reinterpret_cast<unsigned char*>(const_cast<char*>(multiline_description.c_str()));
            }
            unsigned char* pvm_courtesy = NULL;
            std::string multiline_courtesy;
            if ((tagval = hdr.global_taglist().get("COPYRIGHT")))
            {
                multiline_courtesy = tagval;
                str::replace(multiline_courtesy, "\\n", "\n");
                pvm_courtesy = reinterpret_cast<unsigned char*>(const_cast<char*>(multiline_courtesy.c_str()));
            }
            unsigned char* pvm_parameter = NULL;
            std::string multiline_parameter;
            if ((tagval = hdr.global_taglist().get("X-PARAMETER")))
            {
                multiline_parameter = tagval;
                str::replace(multiline_parameter, "\\n", "\n");
                pvm_parameter = reinterpret_cast<unsigned char*>(const_cast<char*>(multiline_parameter.c_str()));
            }
            unsigned char* pvm_comment = NULL;
            std::string multiline_comment;
            if ((tagval = hdr.global_taglist().get("COMMENT")))
            {
                multiline_comment = tagval;
                str::replace(multiline_comment, "\\n", "\n");
                pvm_comment = reinterpret_cast<unsigned char*>(const_cast<char*>(multiline_comment.c_str()));
            }

            try
            {
                writePVMvolume(nameo.c_str(), data.ptr<unsigned char>(),
                        pvm_width, pvm_height, pvm_depth, pvm_components,
                        pvm_scalex, pvm_scaley, pvm_scalez,
                        pvm_description, pvm_courtesy, pvm_parameter, pvm_comment);
            }
            catch (...)
            {
                throw exc(nameo + ": cannot write PVM data");
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
