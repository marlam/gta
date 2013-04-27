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

#include <sndfile.h>

#include "base/msg.h"
#include "base/blb.h"
#include "base/opt.h"
#include "base/str.h"
#include "base/chk.h"

#include "lib.h"

extern "C" void gtatool_to_sndfile_help(void)
{
    msg::req_txt("to-sndfile [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to the WAV audio format via libsndfile.\n"
            "Currently the sample data type must be one of int16, float32, or float64.");
}

extern "C" int gtatool_to_sndfile(int argc, char *argv[])
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
        gtatool_to_sndfile_help();
        return 0;
    }

    try
    {
        std::string nameo = arguments.size() == 1 ? arguments[0] : arguments[1];
        array_loop_t array_loop;
        gta::header hdr;
        std::string name;
	SNDFILE *sndo;
	SF_INFO sfinfo;

        array_loop.start(arguments.size() == 1 ? std::vector<std::string>() : std::vector<std::string>(1, arguments[0]), nameo);
        while (array_loop.read(hdr, name))
        {
            if (hdr.dimensions() != 1)
            {
                throw exc(name + ": only one-dimensional arrays can be converted to audio.");
            }
            gta::type type = hdr.component_type(0);
            if (type != gta::int16 && type != gta::float32 && type != gta::float64)
            {
                throw exc(name + ": component type not supported.");
            }
            for (uintmax_t c = 1; c < hdr.components(); c++)
            {
                if (hdr.component_type(c) != type)
                {
                    throw exc(name + ": component type(s) not supported.");
                }
            }

            std::memset(&sfinfo, 0, sizeof(sfinfo));
            sfinfo.frames = hdr.dimension_size(0);
            {
                double sample_distance;
                if (!hdr.dimension_taglist(0).get("SAMPLE-DISTANCE")
                        || std::sscanf(hdr.dimension_taglist(0).get("SAMPLE-DISTANCE"),
                            "%lg s", &sample_distance) != 1)
                {
                    sample_distance = 1.0 / 44100.0;
                }
                sfinfo.samplerate = ::round(1.0 / sample_distance);
            }
            sfinfo.channels = checked_cast<int>(hdr.components());
            sfinfo.format = SF_FORMAT_WAV | SF_ENDIAN_FILE;
            if (type == gta::int16)
            {
                sfinfo.format |= SF_FORMAT_PCM_16;
            }
            else if (type == gta::float32)
            {
                sfinfo.format |= SF_FORMAT_FLOAT;
            }
            else
            {
                sfinfo.format |= SF_FORMAT_DOUBLE;
            }
            sndo = sf_open(nameo.c_str(), SFM_WRITE, &sfinfo);
            if (!sndo)
            {
                throw exc(nameo + ": cannot open file.");
            }

            element_loop_t element_loop;
            array_loop.start_element_loop(element_loop, hdr, gta::header());
            uintmax_t elements = hdr.elements();
            while (elements > 0)
            {
                uintmax_t n = std::min(elements, static_cast<uintmax_t>(sfinfo.samplerate));
                const void *data = element_loop.read(n);
                uintmax_t c;
                if (type == gta::int16)
                {
                    c = sf_writef_short(sndo, static_cast<const short *>(data), n);
                }
                else if (type == gta::float32)
                {
                    c = sf_writef_float(sndo, static_cast<const float *>(data), n);
                }
                else
                {
                    c = sf_writef_double(sndo, static_cast<const double *>(data), n);
                }
                if (c < n)
                {
                    throw exc(nameo + ": cannot write enough data.");
                }
                elements -= n;
            }
            sf_close(sndo);
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
