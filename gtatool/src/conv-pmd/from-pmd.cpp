/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2014, 2017
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

#define PMD_NO_DEPRECATED
#include <pmdsdk2.h>

#include "base/dbg.h"
#include "base/msg.h"
#include "base/blb.h"
#include "base/opt.h"
#include "base/str.h"
#include "base/end.h"
#include "base/fio.h"

#include "lib.h"


extern "C" void gtatool_from_pmd_help(void)
{
    msg::req_txt(
            "from-pmd -d|--dimensions=w,h <input-file> [output-file]\n"
            "from-pmd [-P|--processing-plugin=<proc>] <input-file> [<output-file>]\n"
            "\n"
            "Converts PMD files (as created by PMDSDK2 and the CamVis software) to GTAs.\n"
            "In the first form (when the -d option given), the data is assumed to originate from the pmdGetSourceData() "
            "function of PMDSDK2 (512 bytes frame header, 512 bytes phase image header, A and B phase info "
            "as big-endian uint16).\n"
            "In the second form (without -d), the data is read using the 'pmdfile' source plugin for PMDSDK2, and the default "
            "processing plugin is camcubeproc.\n"
            "Example: from-pmd camcube-test.pmd camcube-test.gta");
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
    opt::tuple<uintmax_t> dimensions("dimensions", 'd', opt::optional, 1, std::numeric_limits<uintmax_t>::max());
    options.push_back(&dimensions);
    opt::string proc_plugin("processing-plugin", 'P', opt::optional, "camcubeproc");
    options.push_back(&proc_plugin);
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

    try
    {
        if (dimensions.value().size() > 0) {
            const int frame_header_size = 0;
            const int phase_header_size = 512;
            bool host_endianness = (endianness::endianness == endianness::big);
            gta::header hdr;
            hdr.set_dimensions(dimensions.value().size(), &(dimensions.value()[0]));
            hdr.set_components(gta::uint16, gta::uint16);
            hdr.component_taglist(0).set("INTERPRETATION", "X-PMD-PHASE-A");
            hdr.component_taglist(1).set("INTERPRETATION", "X-PMD-PHASE-B");
            blob data(hdr.data_size());
            array_loop_t array_loop;
            array_loop.start(std::vector<std::string>(1, arguments[0]), arguments.size() == 2 ? arguments[1] : "");
            std::string nameo;
            while (fio::has_more(array_loop.file_in()))
            {
                skip_bytes(array_loop.file_in(), array_loop.filename_in(), frame_header_size);
                for (unsigned int i = 0; i < 4; i++)
                {
                    skip_bytes(array_loop.file_in(), array_loop.filename_in(), phase_header_size);
                    array_loop.read_data(hdr, data.ptr());
                    if (!host_endianness)
                    {
                        for (uintmax_t e = 0; e < 2 * hdr.elements(); e++)
                            endianness::swap16(data.ptr<uint16_t>(e));
                    }
                    array_loop.write(hdr, nameo);
                    array_loop.write_data(hdr, data.ptr());
                }
            }
            array_loop.finish();
        } else {
            PMDHandle pmd_hnd;
            PMDDataDescription pmd_dd;
            static const int pmd_str_size = 256;
            char pmd_str[pmd_str_size];

            blob pmd_distances, pmd_amplitudes, pmd_intensities, pmd_flags, pmd_coords;
            struct
            {
                float d, a, i;
                uint8_t f0, f1, f2, f3;
                float x, y, z;
            } gta_element;
            // We assume this struct is tightly packed. Should be true except maybe on very exotic platforms, but check it here:
            if (sizeof(gta_element) != 6 * sizeof(float) + 4 * sizeof(uint8_t))
            {
                throw exc(std::string("Unexpected element structure size. This is a bug! Please report it."));
            }

            // Initialize access: open the file with the 'pmdfile' source plugin,
            // and use the appropriate processing plugin.
            if (pmdOpen(&pmd_hnd, "pmdfile", arguments[0].c_str(), proc_plugin.value().c_str(), "") != PMD_OK)
            {
                pmdGetLastError(0, pmd_str, pmd_str_size);
                throw exc(std::string("Cannot initialize PMD file access: ") + std::string(pmd_str));
            }

            unsigned long frames;
            // Issue commands to the source plugin to set parameters suitable for file processing.
            // The last command writes the number of frames in the file into pmd_str.
            if (pmdSourceCommand(pmd_hnd, pmd_str, pmd_str_size, "SetLoopMode off") != PMD_OK
                    || pmdSourceCommand(pmd_hnd, pmd_str, pmd_str_size, "UseTimestamps false") != PMD_OK
                    || pmdSourceCommand(pmd_hnd, pmd_str, pmd_str_size, "GetNumberOfFrames") != PMD_OK
                    || !str::to(std::string(pmd_str), &frames))
            {
                pmdGetLastError(pmd_hnd, pmd_str, pmd_str_size);
                pmdClose(pmd_hnd);
                throw exc(std::string("PMD source plugin command failed: ") + std::string(pmd_str));
            }

            // Now read and convert all frames
            array_loop_t array_loop;
            array_loop.start(std::vector<std::string>(1, arguments[0]), arguments.size() == 2 ? arguments[1] : "");
            std::string nameo;
            msg::inf(arguments[0] + std::string(": ") + str::from(frames + 1) + " frames");
            for (unsigned long i = 0; i < frames; i++)
            {
                std::string namei = arguments[0] + std::string(" frame ") + str::from(i);
                // Get next frame
                if (pmdUpdate(pmd_hnd) != PMD_OK)
                {
                    pmdGetLastError(pmd_hnd, pmd_str, pmd_str_size);
                    pmdClose(pmd_hnd);
                    throw exc(namei + std::string(": cannot get data"));
                }
                // Get frame info
                if (pmdGetSourceDataDescription(pmd_hnd, &pmd_dd) != PMD_OK)
                {
                    pmdGetLastError(pmd_hnd, pmd_str, pmd_str_size);
                    pmdClose(pmd_hnd);
                    throw exc(namei + std::string(": cannot get data description"));
                }
                if (pmd_dd.subHeaderType != PMD_IMAGE_DATA)
                {
                    pmdClose(pmd_hnd);
                    throw exc(namei + std::string(": frame is not an image"));
                }
                // Get frame-specific properties
                time_t frame_time = pmd_dd.img.timeStampHi;
                unsigned long long frame_microseconds = pmd_dd.img.timeStampLo;
                msg::dbg(namei + std::string(": time stamp ") + str::rfc2822_time(frame_time)
                        + std::string(" plus ") + str::from(frame_microseconds) + " microseconds");
                unsigned int frame_integration_time, frame_modulation_frequency;
                if (pmdGetIntegrationTime(pmd_hnd, &frame_integration_time, 0) != PMD_OK
                        || pmdGetModulationFrequency(pmd_hnd, &frame_modulation_frequency, 0) != PMD_OK)
                {
                    pmdClose(pmd_hnd);
                    throw exc(namei + std::string(": cannot get frame properties"));
                }
                // Get data
                unsigned int w = pmd_dd.img.numColumns;
                unsigned int h = pmd_dd.img.numRows;
                pmd_distances.resize(w, h, sizeof(float));
                pmd_amplitudes.resize(w, h, sizeof(float));
                pmd_intensities.resize(w, h, sizeof(float));
                pmd_flags.resize(w, h, sizeof(unsigned int));
                pmd_coords.resize(w, h, 3 * sizeof(float));
                if (pmdGetDistances(pmd_hnd, pmd_distances.ptr<float>(), pmd_distances.size()) != PMD_OK
                        || pmdGetAmplitudes(pmd_hnd, pmd_amplitudes.ptr<float>(), pmd_amplitudes.size()) != PMD_OK
                        || pmdGetIntensities(pmd_hnd, pmd_intensities.ptr<float>(), pmd_intensities.size()) != PMD_OK
                        || pmdGetFlags(pmd_hnd, pmd_flags.ptr<unsigned int>(), pmd_flags.size()) != PMD_OK
                        || pmdGet3DCoordinates(pmd_hnd, pmd_coords.ptr<float>(), pmd_coords.size()) != PMD_OK)
                {
                    pmdClose(pmd_hnd);
                    throw exc(namei + std::string(": cannot get frame data"));
                }
                // Create a GTA
                gta::header hdr;
                hdr.set_dimensions(w, h);
                gta::type types[] = { gta::float32, gta::float32, gta::float32,
                    gta::uint8, gta::uint8, gta::uint8, gta::uint8,
                    gta::float32, gta::float32, gta::float32 };
                hdr.set_components(10, types);
                hdr.global_taglist().set("DESCRIPTION", "PMD frame");
                hdr.global_taglist().set("DATE", str::rfc2822_time(frame_time).c_str());
                hdr.global_taglist().set("X-PMD-TIMESTAMP-HI", str::from(frame_time).c_str());
                hdr.global_taglist().set("X-PMD-TIMESTAMP-LO", str::from(frame_microseconds).c_str());
                hdr.global_taglist().set("X-PMD-INTEGRATION-TIME", (str::from(frame_integration_time) + std::string(" μs")).c_str());
                hdr.global_taglist().set("X-PMD-MODULATION-FREQUENCY", (str::from(frame_modulation_frequency) + std::string(" Hz")).c_str());
                hdr.component_taglist(0).set("INTERPRETATION", "DISTANCE");
                hdr.component_taglist(1).set("INTERPRETATION", "X-PMD-AMPLITUDE");
                hdr.component_taglist(2).set("INTERPRETATION", "X-PMD-INTENSITY");
                hdr.component_taglist(3).set("INTERPRETATION", "X-PMD-FLAG-INVALID");
                hdr.component_taglist(4).set("INTERPRETATION", "X-PMD-FLAG-SATURATED");
                hdr.component_taglist(5).set("INTERPRETATION", "X-PMD-FLAG-LOW-SIGNAL");
                hdr.component_taglist(6).set("INTERPRETATION", "X-PMD-FLAG-INCONSISTENT");
                hdr.component_taglist(7).set("INTERPRETATION", "X");
                hdr.component_taglist(8).set("INTERPRETATION", "Y");
                hdr.component_taglist(9).set("INTERPRETATION", "Z");
                array_loop.write(hdr, nameo);
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, gta::header(), hdr);
                for (uintmax_t e = 0; e < hdr.elements(); e++)
                {
                    gta_element.d = pmd_distances.ptr<float>()[e];
                    gta_element.a = pmd_amplitudes.ptr<float>()[e];
                    gta_element.i = pmd_intensities.ptr<float>()[e];
                    gta_element.f0 = (pmd_flags.ptr<unsigned int>()[e] & PMD_FLAG_INVALID) ? 0xff : 0x00;
                    gta_element.f1 = (pmd_flags.ptr<unsigned int>()[e] & PMD_FLAG_SATURATED) ? 0xff : 0x00;
                    gta_element.f2 = (pmd_flags.ptr<unsigned int>()[e] & PMD_FLAG_LOW_SIGNAL) ? 0xff : 0x00;
                    gta_element.f3 = (pmd_flags.ptr<unsigned int>()[e] & PMD_FLAG_INCONSISTENT) ? 0xff : 0x00;
                    gta_element.x = pmd_coords.ptr<float>()[3 * e + 0];
                    gta_element.y = pmd_coords.ptr<float>()[3 * e + 1];
                    gta_element.z = pmd_coords.ptr<float>()[3 * e + 2];
                    element_loop.write(&gta_element);
                }
            }
            pmdClose(pmd_hnd);
            array_loop.finish();
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
