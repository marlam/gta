/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2011
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

#include <sndfile.h>

#include "msg.h"
#include "blob.h"
#include "cio.h"
#include "opt.h"
#include "str.h"
#include "intcheck.h"

#include "lib.h"

extern "C" void gtatool_from_sndfile_help(void)
{
    msg::req_txt("from-sndfile [<input-file>] <output-file>\n"
            "\n"
            "Converts audio files that libsndfile can read to GTAs.");
}

extern "C" int gtatool_from_sndfile(int argc, char *argv[])
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
        gtatool_from_sndfile_help();
        return 0;
    }

    try
    {
        array_loop_t array_loop;
        array_loop.start(std::vector<std::string>(1, arguments[0]), arguments.size() == 2 ? arguments[1] : "");

        std::string namei = arguments[0];
	SNDFILE *sndi;
	SF_INFO sfinfo;

        gta::header hdr;
        std::string nameo;

        std::memset(&sfinfo, 0, sizeof(sfinfo));
        sndi = sf_open(namei.c_str(), SFM_READ, &sfinfo);
        if (!sndi)
        {
            throw exc(namei + ": cannot open file.");
        }

        hdr.set_dimensions(sfinfo.frames);
        hdr.dimension_taglist(0).set("INTERPRETATION", "T");
        hdr.dimension_taglist(0).set("X-SAMPLE-RATE", str::from(sfinfo.samplerate).c_str());
        hdr.dimension_taglist(0).set("SAMPLE-DISTANCE", (str::from(1.0 / sfinfo.samplerate) + " s").c_str());
        std::vector<gta::type> types;
        if (sfinfo.format & SF_FORMAT_PCM_S8
                || sfinfo.format & SF_FORMAT_PCM_U8
                || sfinfo.format & SF_FORMAT_PCM_16)
        {
            types.resize(sfinfo.channels, gta::int16);
        }
        else if (sfinfo.format & SF_FORMAT_DOUBLE)
        {
            types.resize(sfinfo.channels, gta::float64);
        }
        else
        {
            // default fallback
            types.resize(sfinfo.channels, gta::float32);
        }
        hdr.set_components(sfinfo.channels, &(types[0]));

        array_loop.write(hdr, nameo);
        blob elementbuf(hdr.element_size(), sfinfo.samplerate);
        element_loop_t element_loop;
        array_loop.start_element_loop(element_loop, gta::header(), hdr);
        uintmax_t elements = hdr.elements();
        while (elements > 0)
        {
            uintmax_t n = std::min(elements, static_cast<uintmax_t>(sfinfo.samplerate));
            uintmax_t c;
            if (hdr.component_type(0) == gta::int16)
            {
                c = sf_readf_short(sndi, elementbuf.ptr<short>(), n);
            }
            else if (hdr.component_type(0) == gta::float32)
            {
                c = sf_readf_float(sndi, elementbuf.ptr<float>(), n);
            }
            else
            {
                c = sf_readf_double(sndi, elementbuf.ptr<double>(), n);
            }
            if (c < n)
            {
                throw exc(namei + ": cannot read enough data.");
            }
            element_loop.write(elementbuf.ptr(), n);
            elements -= n;
        }
        sf_close(sndi);
        array_loop.finish();
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
