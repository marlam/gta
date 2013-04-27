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

#include <gta/gta.hpp>

#include "base/msg.h"
#include "base/str.h"
#include "base/exc.h"
#include "base/opt.h"
#include "base/chk.h"
#include "base/blb.h"

#include "lib.h"

#include "ddsbase.h"


extern "C" void gtatool_from_pvm_help(void)
{
    msg::req_txt("from-pvm <input-file> [<output-file>]\n"
            "\n"
            "Converts pvm files to GTAs.");
}

extern "C" int gtatool_from_pvm(int argc, char *argv[])
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
        gtatool_from_pvm_help();
        return 0;
    }

    try
    {
        array_loop_t array_loop;
        array_loop.start(std::vector<std::string>(1, arguments[0]), arguments.size() == 2 ? arguments[1] : "");

        std::string namei = arguments[0];
        unsigned int pvm_width = 0;
        unsigned int pvm_height = 0;
        unsigned int pvm_depth = 0;
        unsigned int pvm_components = 0;
        float pvm_scalex = 1.0f;
        float pvm_scaley = 1.0f;
        float pvm_scalez = 1.0f;
        unsigned char* pvm_description = NULL;
        unsigned char* pvm_courtesy = NULL;
        unsigned char* pvm_parameter = NULL;
        unsigned char* pvm_comment = NULL;
        unsigned char* pvm_data = NULL;

        try
        {
            pvm_data = readPVMvolume(namei.c_str(),
                    &pvm_width, &pvm_height, &pvm_depth, &pvm_components,
                    &pvm_scalex, &pvm_scaley, &pvm_scalez,
                    &pvm_description, &pvm_courtesy, &pvm_parameter, &pvm_comment);
        }
        catch (...)
        {
            // if readPVMvolume() throws an exception, then pvm_data is NULL
        }
        if (!pvm_data)
        {
            throw exc(namei + ": cannot read PVM data");
        }

        gta::header hdr;
        std::string nameo;

        hdr.set_dimensions(pvm_width, pvm_height, pvm_depth);
        hdr.set_components(pvm_components == 2 ? gta::uint16 : gta::uint8);
        if (pvm_scalex < 1.0f || pvm_scalex > 1.0f)
            hdr.dimension_taglist(0).set("SAMPLE-DISTANCE", str::from(pvm_scalex).c_str());
        if (pvm_scaley < 1.0f || pvm_scaley > 1.0f)
            hdr.dimension_taglist(1).set("SAMPLE-DISTANCE", str::from(pvm_scaley).c_str());
        if (pvm_scalez < 1.0f || pvm_scalez > 1.0f)
            hdr.dimension_taglist(2).set("SAMPLE-DISTANCE", str::from(pvm_scalez).c_str());
        if (pvm_description)
        {
            std::string oneline(reinterpret_cast<char*>(pvm_description));
            oneline = str::replace(oneline, "\n", "\\n");
            try { hdr.global_taglist().set("DESCRIPTION", oneline.c_str()); }
            catch (...) { msg::wrn("cannot set DESCRIPTION tag"); }
        }
        if (pvm_courtesy)
        {
            std::string oneline(reinterpret_cast<char*>(pvm_courtesy));
            oneline = str::replace(oneline, "\n", "\\n");
            try { hdr.global_taglist().set("COPYRIGHT", oneline.c_str()); }
            catch (...) { msg::wrn("cannot set COPYRIGHT tag"); }
        }
        if (pvm_parameter)
        {
            std::string oneline(reinterpret_cast<char*>(pvm_parameter));
            oneline = str::replace(oneline, "\n", "\\n");
            try { hdr.global_taglist().set("X-PARAMETER", oneline.c_str()); }
            catch (...) { msg::wrn("cannot set X-PARAMETER tag"); }
        }
        if (pvm_comment)
        {
            std::string oneline(reinterpret_cast<char*>(pvm_comment));
            oneline = str::replace(oneline, "\n", "\\n");
            try { hdr.global_taglist().set("COMMENT", oneline.c_str()); }
            catch (...) { msg::wrn("cannot set COMMENT tag"); }
        }

        array_loop.write(hdr, nameo);
        array_loop.write_data(hdr, pvm_data);
        array_loop.finish();
        free(pvm_data);
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
