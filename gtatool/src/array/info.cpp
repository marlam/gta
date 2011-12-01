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

#include <sstream>
#include <cstdio>
#include <cctype>
#include <cmath>

#include <gta/gta.hpp>

#include "msg.h"
#include "opt.h"
#include "cio.h"
#include "str.h"

#include "lib.h"


extern "C" void gtatool_info_help(void)
{
    msg::req_txt(
            "info [-s|--statistics] [<files...>]\n"
            "\n"
            "Print information about GTAs.\n"
            "If --statistics is given, simple statistics about the values in each component "
            "are computed and printed (in double precision, regardless of input type). Values "
            "that are not finite numbers are ignored.");
}

extern "C" int gtatool_info(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::flag statistics("statistics", 's', opt::optional);
    options.push_back(&statistics);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_info_help();
        return 0;
    }

    try
    {
        array_loop_t array_loop;
        gta::header hdr;
        std::string name;
        array_loop.start(arguments, "");
        std::vector<double> minima;
        std::vector<double> maxima;
        std::vector<double> sum;
        std::vector<double> squaresum;
        while (array_loop.read(hdr, name))
        {
            if (statistics.value() && !hdr.data_size() == 0)
            {
                minima.clear();
                maxima.clear();
                sum.clear();
                squaresum.clear();
                minima.resize(checked_cast<size_t>(hdr.components()), +std::numeric_limits<double>::max());
                maxima.resize(checked_cast<size_t>(hdr.components()), -std::numeric_limits<double>::max());
                sum.resize(checked_cast<size_t>(hdr.components()), 0.0);
                squaresum.resize(checked_cast<size_t>(hdr.components()), 0.0);
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, hdr, hdr);
                for (uintmax_t e = 0; e < hdr.elements(); e++)
                {
                    const void *element = element_loop.read();
                    for (uintmax_t c = 0; c < hdr.components(); c++)
                    {
                        const void *component = hdr.component(element, c);
                        double val;
                        switch (hdr.component_type(c))
                        {
                        case gta::int8:
                            {
                                int8_t v;
                                memcpy(&v, component, sizeof(int8_t));
                                val = v;
                            }
                            break;
                        case gta::uint8:
                            {
                                uint8_t v;
                                memcpy(&v, component, sizeof(uint8_t));
                                val = v;
                            }
                            break;
                        case gta::int16:
                            {
                                int16_t v;
                                memcpy(&v, component, sizeof(int16_t));
                                val = v;
                            }
                            break;
                        case gta::uint16:
                            {
                                uint16_t v;
                                memcpy(&v, component, sizeof(uint16_t));
                                val = v;
                            }
                            break;
                        case gta::int32:
                            {
                                int32_t v;
                                memcpy(&v, component, sizeof(int32_t));
                                val = v;
                            }
                            break;
                        case gta::uint32:
                            {
                                uint32_t v;
                                memcpy(&v, component, sizeof(uint32_t));
                                val = v;
                            }
                            break;
                        case gta::int64:
                            {
                                int64_t v;
                                memcpy(&v, component, sizeof(int64_t));
                                val = v;
                            }
                            break;
                        case gta::uint64:
                            {
                                uint64_t v;
                                memcpy(&v, component, sizeof(uint64_t));
                                val = v;
                            }
                            break;
                        case gta::float32:
                            {
                                float v;
                                memcpy(&v, component, sizeof(float));
                                val = v;
                            }
                            break;
                        case gta::float64:
                            {
                                memcpy(&val, component, sizeof(double));
                            }
                            break;
                        case gta::cfloat32:
                            {
                                float v[2];
                                memcpy(v, component, 2 * sizeof(float));
                                val = std::sqrt(v[0] * v[0] + v[1] * v[1]);
                            }
                            break;
                        case gta::cfloat64:
                            {
                                double v[2];
                                memcpy(v, component, 2 * sizeof(double));
                                val = std::sqrt(v[0] * v[0] + v[1] * v[1]);
                            }
                            break;
                        default:
                            throw exc(std::string("cannot compute minimum/maximum for component type ")
                                    + type_to_string(hdr.component_type(c), hdr.component_size(c)));
                            break;
                        }
                        if (std::isfinite(val))
                        {
                            if (val < minima[c])
                                minima[c] = val;
                            else if (val > maxima[c])
                                maxima[c] = val;
                            sum[c] += val;
                            squaresum[c] += val * val;
                        }
                    }
                }
            }
            else
            {
                array_loop.skip_data(hdr);
            }
            std::stringstream dimensions;
            for (uintmax_t i = 0; i < hdr.dimensions(); i++)
            {
                dimensions << hdr.dimension_size(i);
                if (i < hdr.dimensions() - 1)
                {
                    dimensions << "x";
                }
            }
            std::stringstream components;
            for (uintmax_t i = 0; i < hdr.components(); i++)
            {
                components << type_to_string(hdr.component_type(i), hdr.component_size(i));
                if (i < hdr.components() - 1)
                {
                    components << ",";
                }
            }
            if (hdr.data_size() == 0)
            {
                msg::req(name + ":");
            }
            else
            {
                msg::req(name + ": "
                        + str::from(hdr.data_size()) + " bytes ("
                        + str::human_readable_memsize(hdr.data_size()) + ")");
            }
            msg::req(4, std::string("compression: ") +
                    (hdr.compression() == gta::none ? "none"
                     : hdr.compression() == gta::zlib ? "zlib default level"
                     : hdr.compression() == gta::bzip2 ? "bzip2"
                     : hdr.compression() == gta::xz ? "xz"
                     : hdr.compression() == gta::zlib1 ? "zlib level 1"
                     : hdr.compression() == gta::zlib2 ? "zlib level 2"
                     : hdr.compression() == gta::zlib3 ? "zlib level 3"
                     : hdr.compression() == gta::zlib4 ? "zlib level 4"
                     : hdr.compression() == gta::zlib5 ? "zlib level 5"
                     : hdr.compression() == gta::zlib6 ? "zlib level 6"
                     : hdr.compression() == gta::zlib7 ? "zlib level 7"
                     : hdr.compression() == gta::zlib8 ? "zlib level 8"
                     : hdr.compression() == gta::zlib9 ? "zlib level 9" : "unknown"));
            if (hdr.data_size() == 0)
            {
                msg::req(4, "empty array");
            }
            else
            {
                msg::req(4, dimensions.str() + " elements of type " + components.str());
            }
            for (uintmax_t i = 0; i < hdr.global_taglist().tags(); i++)
            {
                msg::req(8, from_utf8(hdr.global_taglist().name(i)) + "=" + from_utf8(hdr.global_taglist().value(i)));
            }
            for (uintmax_t i = 0; i < hdr.dimensions(); i++)
            {
                msg::req(4, std::string("dimension ") + str::from(i) + ": " + str::from(hdr.dimension_size(i)));
                for (uintmax_t j = 0; j < hdr.dimension_taglist(i).tags(); j++)
                {
                    msg::req(8, from_utf8(hdr.dimension_taglist(i).name(j)) + "=" + from_utf8(hdr.dimension_taglist(i).value(j)));
                }
            }
            for (uintmax_t i = 0; i < hdr.components(); i++)
            {
                msg::req(4, std::string("element component ") + str::from(i) + ": "
                        + type_to_string(hdr.component_type(i), hdr.component_size(i)) + ", "
                        + str::human_readable_memsize(hdr.component_size(i)));
                if (statistics.value() && !hdr.data_size() == 0)
                {
                    msg::req(8, std::string("minimum value = ") + str::from(minima[i]));
                    msg::req(8, std::string("maximum value = ") + str::from(maxima[i]));
                    double mean = sum[i] / (hdr.elements() - 1);
                    double variance = (squaresum[i] - sum[i] / hdr.elements() * sum[i]) / (hdr.elements() - 1);
                    double deviation = std::sqrt(variance);
                    msg::req(8, std::string("sample mean = ") + str::from(mean));
                    msg::req(8, std::string("sample variance = ") + str::from(variance));
                    msg::req(8, std::string("sample deviation = ") + str::from(deviation));
                }
                for (uintmax_t j = 0; j < hdr.component_taglist(i).tags(); j++)
                {
                    msg::req(8, from_utf8(hdr.component_taglist(i).name(j)) + "=" + from_utf8(hdr.component_taglist(i).value(j)));
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
