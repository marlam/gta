/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2013
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
#include <cmath>

#include <gta/gta.hpp>

#include "base/msg.h"
#include "base/blb.h"
#include "base/fio.h"
#include "base/opt.h"
#include "base/chk.h"
#include "base/end.h"

#include "lib.h"


extern "C" void gtatool_from_pmd_help(void)
{
    msg::req_txt("from-pmd -d|--dimensions=<d0,d1,...>\n"
            "    [-F|--frame-header-size=<F>]\n"
            "    [-P|--phase-header-size=<H>]\n"
            "    [-O|--optional-data-size=<O>]\n"
            "    [-c|--compute-results=<MODFREQ>]\n"
            "    <input-file> [<output-file>]\n"
            "\n"
            "Converts PMD files (as created by the PMDSDK and by CamVis) to GTAs.\n"
            "The frame dimensions must be given. The sizes of the frame header (default 144), the phase "
            "header (default 512), and the optional data blob (default 0) can be specified, too. See the "
            "\"CamCube 2.0 Source data format\" documentation from PMDTec for details.\n"
            "By default, each phase image will be converted to one array with two channels (A and B).\n"
            "If the --compute-results option is given with the modulation frequency in Hz, then four "
            "consecutive phase images will be combined into a single array, and three additional channels "
            "containing the PMD depth, amplitude, and intensity values will be computed.\n"
            "Example: from-pmd -d 200,200 -O 1234 camcube-test.pmd camcube-test.gta");
}

static void skip_bytes(FILE* f, const std::string& n, uintmax_t b)
{
    const uintmax_t N = 1024;
    unsigned char trash[N];
    while (b >= N)
    {
        fio::read(trash, 1, N, f, n);
        b -= N;
    }
    if (b > 0)
    {
        fio::read(trash, 1, b, f, n);
    }
}

extern "C" int gtatool_from_pmd(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::tuple<uintmax_t> dimensions("dimensions", 'd', opt::required, 1, std::numeric_limits<uintmax_t>::max());
    options.push_back(&dimensions);
    opt::val<uintmax_t> frame_header_size("frame-header-size", 'F', opt::optional, 144);
    options.push_back(&frame_header_size);
    opt::val<uintmax_t> phase_header_size("phase-header-size", 'P', opt::optional, 512);
    options.push_back(&phase_header_size);
    opt::val<uintmax_t> optional_data_size("optional-data-size", 'O', opt::optional, 0);
    options.push_back(&optional_data_size);
    opt::val<uintmax_t> compute_results("compute-results", 'c', opt::optional, 1, std::numeric_limits<uintmax_t>::max());
    options.push_back(&compute_results);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, 2, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_from_pmd_help();
        return 0;
    }

    bool host_endianness = (endianness::endianness == endianness::big);
    const double c = 299792458.0; /* Speed of light in m/s */
    float frac_c_modfreq = c / compute_results.value();

    try
    {
        gta::header hdr;
        blob data;
        hdr.set_dimensions(dimensions.value().size(), &(dimensions.value()[0]));
        std::vector<gta::type> comp_types;
        if (compute_results.value())
        {
            comp_types.resize(8, gta::uint16);
            comp_types.resize(11, gta::float32);
            hdr.set_components(comp_types.size(), &(comp_types[0]));
            hdr.component_taglist(0).set("INTERPRETATION", "X-PMD-PHASE0-A");
            hdr.component_taglist(1).set("INTERPRETATION", "X-PMD-PHASE0-B");
            hdr.component_taglist(2).set("INTERPRETATION", "X-PMD-PHASE1-A");
            hdr.component_taglist(3).set("INTERPRETATION", "X-PMD-PHASE1-B");
            hdr.component_taglist(4).set("INTERPRETATION", "X-PMD-PHASE2-A");
            hdr.component_taglist(5).set("INTERPRETATION", "X-PMD-PHASE2-B");
            hdr.component_taglist(6).set("INTERPRETATION", "X-PMD-PHASE3-A");
            hdr.component_taglist(7).set("INTERPRETATION", "X-PMD-PHASE3-B");
            //hdr.component_taglist(8).set("INTERPRETATION", "X-PMD-DEPTH");
            hdr.component_taglist(8).set("INTERPRETATION", "DEPTH/RADIAL");
            hdr.component_taglist(8).set("UNITS", "m");
            hdr.component_taglist(9).set("INTERPRETATION", "X-PMD-AMPLITUDE");
            hdr.component_taglist(10).set("INTERPRETATION", "X-PMD-INTENSITY");
            data.resize(checked_cast<size_t>(hdr.data_size()));
        }
        else
        {
            comp_types.resize(2, gta::uint16);
            hdr.set_components(comp_types.size(), &(comp_types[0]));
            hdr.component_taglist(0).set("INTERPRETATION", "X-PMD-PHASE-A");
            hdr.component_taglist(1).set("INTERPRETATION", "X-PMD-PHASE-B");
        }

        gta::header phase_hdr;
        phase_hdr.set_dimensions(dimensions.value().size(), &(dimensions.value()[0]));
        phase_hdr.set_components(gta::uint16, gta::uint16);
        blob phase_data(phase_hdr.data_size());

        array_loop_t array_loop;
        array_loop.start(std::vector<std::string>(1, arguments[0]), arguments.size() == 2 ? arguments[1] : "");
        std::string nameo;

        while (fio::has_more(array_loop.file_in()))
        {
            skip_bytes(array_loop.file_in(), array_loop.filename_in(), frame_header_size.value());
            for (unsigned int i = 0; i < 4; i++)
            {
                skip_bytes(array_loop.file_in(), array_loop.filename_in(), phase_header_size.value());
                array_loop.read_data(phase_hdr, phase_data.ptr());
                if (!host_endianness)
                {
                    for (uintmax_t e = 0; e < 2 * phase_hdr.elements(); e++)
                        endianness::swap16(phase_data.ptr<uint16_t>(e));
                }
                if (compute_results.value())
                {
                    size_t esu16 = hdr.element_size() / sizeof(uint16_t);
                    for (uintmax_t e = 0; e < phase_hdr.elements(); e++)
                    {
                        data.ptr<uint16_t>()[e * esu16 + 2 * i + 0] = phase_data.ptr<uint16_t>()[2 * e + 0];
                        data.ptr<uint16_t>()[e * esu16 + 2 * i + 1] = phase_data.ptr<uint16_t>()[2 * e + 1];
                    }
                }
                else
                {
                    array_loop.write(hdr, nameo);
                    array_loop.write_data(hdr, phase_data.ptr());
                }
            }
            skip_bytes(array_loop.file_in(), array_loop.filename_in(), optional_data_size.value());
            if (compute_results.value())
            {
                size_t esu16 = hdr.element_size() / sizeof(uint16_t);
                size_t esf32 = hdr.element_size() / sizeof(float);
                for (uintmax_t e = 0; e < hdr.elements(); e++)
                {
                    float D[4];
                    for (int i = 0; i < 4; i++)
                    {
                        float A = *(data.ptr<uint16_t>(e * esu16 + 2 * i + 0));
                        float B = *(data.ptr<uint16_t>(e * esu16 + 2 * i + 1));
                        D[i] = A - B;
                    }
                    float pmd_depth = 0.0f;
                    if (std::abs(D[0] - D[2]) > 0.0f || std::abs(D[1] - D[3]) > 0.0f) {
                        float phase_shift = std::atan2(D[0] - D[2], D[1] - D[3]) - static_cast<float>(M_PI) / 2.0f;
                        if (phase_shift < 0.0f)
                            phase_shift += 2.0f * static_cast<float>(M_PI);
                        pmd_depth = frac_c_modfreq * phase_shift / (4.0f * static_cast<float>(M_PI));
                    }
                    float pmd_amp = std::sqrt((D[0] - D[2]) * (D[0] - D[2]) + (D[1] - D[3]) * (D[1] - D[3]))
                        * static_cast<float>(M_PI) / 2.0f;
                    float pmd_intensity = (D[0] + D[1] + D[2] + D[3]) / 2.0f;
                    data.ptr<float>()[e * esf32 + 4] = pmd_depth;
                    data.ptr<float>()[e * esf32 + 5] = pmd_amp;
                    data.ptr<float>()[e * esf32 + 6] = pmd_intensity;
                }
                array_loop.write(hdr, nameo);
                array_loop.write_data(hdr, data.ptr());
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
