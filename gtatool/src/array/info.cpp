/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014
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

#include "base/msg.h"
#include "base/opt.h"
#include "base/str.h"

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
        std::vector<uintmax_t> valid_values;
        while (array_loop.read(hdr, name))
        {
            if (statistics.value() && hdr.data_size() != 0)
            {
                minima.clear();
                maxima.clear();
                sum.clear();
                squaresum.clear();
                valid_values.clear();
                minima.resize(checked_cast<size_t>(hdr.components()), +std::numeric_limits<double>::max());
                maxima.resize(checked_cast<size_t>(hdr.components()), -std::numeric_limits<double>::max());
                sum.resize(checked_cast<size_t>(hdr.components()), 0.0);
                squaresum.resize(checked_cast<size_t>(hdr.components()), 0.0);
                valid_values.resize(checked_cast<size_t>(hdr.components()), 0);
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, hdr, hdr);
                for (uintmax_t e = 0; e < hdr.elements(); e++)
                {
                    const void *element = element_loop.read();
                    for (uintmax_t c = 0; c < hdr.components(); c++)
                    {
                        union
                        {
                            int8_t int8_v;
                            uint8_t uint8_v;
                            int16_t int16_v;
                            uint16_t uint16_v;
                            int32_t int32_v;
                            uint32_t uint32_v;
                            int64_t int64_v;
                            uint64_t uint64_v;
#ifdef HAVE_INT128_T
                            int128_t int128_v;
#endif
#ifdef HAVE_UINT128_T
                            uint128_t uint128_v;
#endif
                            float float_v;
                            double double_v;
#ifdef HAVE_FLOAT128_T
                            float128_t float128_v;
#endif
                        } nodata_value;
                        bool have_nodata_value = false;
                        const char* tagval = hdr.component_taglist(c).get("NO_DATA_VALUE");
                        if (tagval)
                        {
                            switch (hdr.component_type(c))
                            {
                            case gta::int8:
                                have_nodata_value = (str::to(tagval, &nodata_value.int8_v) == 0);
                                break;
                            case gta::uint8:
                                have_nodata_value = (str::to(tagval, &nodata_value.uint8_v) == 0);
                                break;
                            case gta::int16:
                                have_nodata_value = (str::to(tagval, &nodata_value.int16_v) == 0);
                                break;
                            case gta::uint16:
                                have_nodata_value = (str::to(tagval, &nodata_value.uint16_v) == 0);
                                break;
                            case gta::int32:
                                have_nodata_value = (str::to(tagval, &nodata_value.int32_v) == 0);
                                break;
                            case gta::uint32:
                                have_nodata_value = (str::to(tagval, &nodata_value.uint32_v) == 0);
                                break;
                            case gta::int64:
                                have_nodata_value = (str::to(tagval, &nodata_value.int64_v) == 0);
                                break;
                            case gta::uint64:
                                have_nodata_value = (str::to(tagval, &nodata_value.uint64_v) == 0);
                                break;
#ifdef HAVE_INT128_T
                            case gta::int128:
                                have_nodata_value = (str::to(tagval, &nodata_value.int128_v) == 0);
                                break;
#endif
#ifdef HAVE_UINT128_T
                            case gta::uint128:
                                have_nodata_value = (str::to(tagval, &nodata_value.uint128_v) == 0);
                                break;
#endif
                            case gta::float32:
                            case gta::cfloat32:
                                have_nodata_value = (str::to(tagval, &nodata_value.float_v) == 0);
                                break;
                            case gta::float64:
                            case gta::cfloat64:
                                have_nodata_value = (str::to(tagval, &nodata_value.double_v) == 0);
                                break;
#ifdef HAVE_FLOAT128_T
                            case gta::float128:
                            case gta::cfloat128:
                                have_nodata_value = (str::to(tagval, &nodata_value.double_v) == 0);
                                break;
#endif
                            default:
                                throw exc(std::string("cannot handle NO_DATA_VALUE for component type ")
                                        + type_to_string(hdr.component_type(c), hdr.component_size(c)));
                                break;
                            }
                        }
                        const void *component = hdr.component(element, c);
                        double val = std::numeric_limits<double>::quiet_NaN();
                        switch (hdr.component_type(c))
                        {
                        case gta::int8:
                            {
                                int8_t v;
                                memcpy(&v, component, sizeof(int8_t));
                                if (!have_nodata_value || v != nodata_value.int8_v)
                                    val = v;
                            }
                            break;
                        case gta::uint8:
                            {
                                uint8_t v;
                                memcpy(&v, component, sizeof(uint8_t));
                                if (!have_nodata_value || v != nodata_value.uint8_v)
                                    val = v;
                            }
                            break;
                        case gta::int16:
                            {
                                int16_t v;
                                memcpy(&v, component, sizeof(int16_t));
                                if (!have_nodata_value || v != nodata_value.int16_v)
                                    val = v;
                            }
                            break;
                        case gta::uint16:
                            {
                                uint16_t v;
                                memcpy(&v, component, sizeof(uint16_t));
                                if (!have_nodata_value || v != nodata_value.uint16_v)
                                    val = v;
                            }
                            break;
                        case gta::int32:
                            {
                                int32_t v;
                                memcpy(&v, component, sizeof(int32_t));
                                if (!have_nodata_value || v != nodata_value.int32_v)
                                    val = v;
                            }
                            break;
                        case gta::uint32:
                            {
                                uint32_t v;
                                memcpy(&v, component, sizeof(uint32_t));
                                if (!have_nodata_value || v != nodata_value.uint32_v)
                                    val = v;
                            }
                            break;
                        case gta::int64:
                            {
                                int64_t v;
                                memcpy(&v, component, sizeof(int64_t));
                                if (!have_nodata_value || v != nodata_value.int64_v)
                                    val = v;
                            }
                            break;
                        case gta::uint64:
                            {
                                uint64_t v;
                                memcpy(&v, component, sizeof(uint64_t));
                                if (!have_nodata_value || v != nodata_value.uint64_v)
                                    val = v;
                            }
                            break;
#ifdef HAVE_INT128_T
                        case gta::int128:
                            {
                                int128_t v;
                                memcpy(&v, component, sizeof(int128_t));
                                if (!have_nodata_value || v != nodata_value.int128_v)
                                    val = v;
                            }
                            break;
#endif
#ifdef HAVE_UINT128_T
                        case gta::uint128:
                            {
                                uint128_t v;
                                memcpy(&v, component, sizeof(uint128_t));
                                if (!have_nodata_value || v != nodata_value.uint128_v)
                                    val = v;
                            }
                            break;
#endif
                        case gta::float32:
                        case gta::cfloat32:
                            {
                                float v;
                                memcpy(&v, component, sizeof(float));
                                if (!have_nodata_value || std::memcmp(&v, &nodata_value.float_v, sizeof(float)) != 0)
                                    val = v;
                            }
                            break;
                        case gta::float64:
                        case gta::cfloat64:
                            {
                                double v;
                                memcpy(&v, component, sizeof(double));
                                if (!have_nodata_value || std::memcmp(&v, &nodata_value.double_v, sizeof(double)) != 0)
                                    val = v;
                            }
                            break;
#ifdef HAVE_FLOAT128_T
                        case gta::float128:
                        case gta::cfloat128:
                            {
                                float128_t v;
                                memcpy(&v, component, sizeof(float128_t));
                                if (!have_nodata_value || std::memcmp(&v, &nodata_value.double_v, sizeof(float128_t)) != 0)
                                    val = v;
                            }
                            break;
#endif
                        default:
                            throw exc(std::string("cannot compute minimum/maximum for component type ")
                                    + type_to_string(hdr.component_type(c), hdr.component_size(c)));
                            break;
                        }
                        if (std::isfinite(val))
                        {
                            if (valid_values[c] == 0)
                            {
                                minima[c] = val;
                                maxima[c] = val;
                                sum[c] = val;
                                squaresum[c] = val * val;
                            }
                            else
                            {
                                if (val < minima[c])
                                    minima[c] = val;
                                else if (val > maxima[c])
                                    maxima[c] = val;
                                sum[c] += val;
                                squaresum[c] += val * val;
                            }
                            valid_values[c]++;
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
            if (hdr.dimensions() == 0)
            {
                dimensions << "0";
            }
            else if (hdr.dimensions() > 1)
            {
                dimensions << " = " << hdr.elements();
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
            else if (hdr.data_size() < 1024)
            {
                msg::req(name + ": "
                        + str::from(hdr.data_size()) + " bytes");
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
            if (hdr.components() > 0)
            {
                msg::req(4, dimensions.str() + " elements of type " + components.str());
            }
            else
            {
                msg::req(4, dimensions.str() + " empty elements");
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
                if (statistics.value() && hdr.data_size() != 0)
                {
                    msg::req(8, std::string("minimum value = ") + (valid_values[i] > 0 ? str::from(minima[i]) : "unavailable"));
                    msg::req(8, std::string("maximum value = ") + (valid_values[i] > 0 ? str::from(maxima[i]) : "unavailable"));
                    double mean = 0.0, variance = 0.0, deviation = 0.0;
                    if (valid_values[i] > 0) {
                        mean = sum[i] / valid_values[i];
                    }
                    if (valid_values[i] > 1) {
                        variance = (squaresum[i] - sum[i] / hdr.elements() * sum[i]) / (valid_values[i] - 1);
                        deviation = std::sqrt(variance);
                    }
                    msg::req(8, std::string("sample mean = ") + (valid_values[i] > 0 ? str::from(mean) : "unavailable"));
                    msg::req(8, std::string("sample variance = ") + (valid_values[i] > 1 ? str::from(variance) : "unavailable"));
                    msg::req(8, std::string("sample deviation = ") + (valid_values[i] > 1 ? str::from(deviation) : "unavailable"));
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
