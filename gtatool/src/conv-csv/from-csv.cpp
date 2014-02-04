/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2012, 2013, 2014
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
#include <cstdio>
#include <cstdlib>
#include <cerrno>

#include <gta/gta.hpp>

#include "base/msg.h"
#include "base/str.h"
#include "base/fio.h"
#include "base/opt.h"
#include "base/chk.h"

#include "lib.h"

#include "delimiter.h"


extern "C" void gtatool_from_csv_help(void)
{
    msg::req_txt("from-csv [-c|--components=<c0,c1,...>] [-D|--delimiter=D]\n"
            "    [-N|--no-data-value=<n0,n1,...>] <input-file> [<output-file>]\n"
            "\n"
            "Converts CSV files to GTAs. By default, each array element has one component of type float64. "
            "This can be changed with the -c option. Supported component types are all integer types, "
            "float32, and float64.\n"
            "The delimiter D must be a single ASCII character; the default is to autodetect it.\n"
            "Blank lines in the input file are interpreted as separators between different arrays.\n"
            "Example: from-csv -c uint8,uint8,uint8 rgb.csv rgb.gta");
}

static bool parse_component(const std::string& s, gta::type t, void* c)
{
    bool ok = false;
    errno = 0;
    if (t == gta::int8)
    {
        int8_t v;
        long xv = strtol(s.c_str(), NULL, 0);
        ok = (errno == 0
                && xv >= std::numeric_limits<int8_t>::min()
                && xv <= std::numeric_limits<int8_t>::max());
        v = xv;
        std::memcpy(c, &v, sizeof(int8_t));
    }
    else if (t == gta::uint8)
    {
        uint8_t v;
        unsigned long xv = strtoul(s.c_str(), NULL, 0);
        ok = (errno == 0 && xv <= std::numeric_limits<uint8_t>::max());
        v = xv;
        std::memcpy(c, &v, sizeof(uint8_t));
    }
    else if (t == gta::int16)
    {
        int16_t v;
        long xv = strtol(s.c_str(), NULL, 0);
        ok = (errno == 0
                && xv >= std::numeric_limits<int16_t>::min()
                && xv <= std::numeric_limits<int16_t>::max());
        v = xv;
        std::memcpy(c, &v, sizeof(int16_t));
    }
    else if (t == gta::uint16)
    {
        uint16_t v;
        unsigned long xv = strtoul(s.c_str(), NULL, 0);
        ok = (errno == 0 && xv <= std::numeric_limits<uint16_t>::max());
        v = xv;
        std::memcpy(c, &v, sizeof(uint16_t));
    }
    else if (t == gta::int32)
    {
        int32_t v;
        long xv = strtol(s.c_str(), NULL, 0);
        ok = (errno == 0
                && xv >= std::numeric_limits<int32_t>::min()
                && xv <= std::numeric_limits<int32_t>::max());
        v = xv;
        std::memcpy(c, &v, sizeof(int32_t));
    }
    else if (t == gta::uint32)
    {
        uint32_t v;
        unsigned long xv = strtoul(s.c_str(), NULL, 0);
        ok = (errno == 0 && xv <= std::numeric_limits<uint32_t>::max());
        v = xv;
        std::memcpy(c, &v, sizeof(uint32_t));
    }
    else if (t == gta::int64)
    {
        int64_t v;
        long long xv = strtoll(s.c_str(), NULL, 0);
        ok = (errno == 0
                && xv >= std::numeric_limits<int64_t>::min()
                && xv <= std::numeric_limits<int64_t>::max());
        v = xv;
        std::memcpy(c, &v, sizeof(int64_t));
    }
    else if (t == gta::uint64)
    {
        uint64_t v;
        unsigned long long xv = strtoul(s.c_str(), NULL, 0);
        ok = (errno == 0 && xv <= std::numeric_limits<uint64_t>::max());
        v = xv;
        std::memcpy(c, &v, sizeof(uint64_t));
    }
    else if (t == gta::float32)
    {
        float v;
        errno = 0;
        v = strtof(s.c_str(), NULL);
        ok = (errno == 0);
        std::memcpy(c, &v, sizeof(float));
    }
    else // t == gta::float64
    {
        double v;
        errno = 0;
        v = std::strtod(s.c_str(), NULL);
        ok = (errno == 0);
        std::memcpy(c, &v, sizeof(double));
    }
    return ok;
}

extern "C" int gtatool_from_csv(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::string components("components", 'c', opt::optional);
    options.push_back(&components);
    std::vector<std::string> delimiters = gta_csv_create_delimiters();
    opt::string delimiter("delimiter", 'D', opt::optional, delimiters, std::string());
    options.push_back(&delimiter);
    opt::string no_data_value("no-data-value", 'N', opt::optional);
    options.push_back(&no_data_value);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, 2, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_from_csv_help();
        return 0;
    }
    std::string delim = delimiter.value();

    try
    {
        std::string nameo;
        gta::header hdr;

        std::vector<gta::type> comp_types;
        std::vector<uintmax_t> comp_sizes;
        if (!components.values().empty())
        {
            typelist_from_string(components.value(), &comp_types, &comp_sizes);
            for (size_t c = 0; c < comp_types.size(); c++)
            {
                if (comp_types[c] != gta::int8
                        && comp_types[c] != gta::uint8
                        && comp_types[c] != gta::int16
                        && comp_types[c] != gta::uint16
                        && comp_types[c] != gta::int32
                        && comp_types[c] != gta::uint32
                        && comp_types[c] != gta::int64
                        && comp_types[c] != gta::uint64
                        && comp_types[c] != gta::float32
                        && comp_types[c] != gta::float64)
                {
                    throw exc(std::string("unsupported element component type."));
                }
            }
        }
        else
        {
            comp_types.push_back(gta::float64);
        }
        hdr.set_components(comp_types.size(), &(comp_types[0]), NULL);
        blob no_data_element(checked_cast<size_t>(hdr.element_size()));
        if (no_data_value.value().empty())
        {
            memset(no_data_element.ptr(), 0, hdr.element_size());
        }
        else
        {
            valuelist_from_string(no_data_value.value(), comp_types, comp_sizes, no_data_element.ptr());
        }

        std::string namei = arguments[0];
        FILE *fi = fio::open(namei, "r");

        FILE *ft;
        std::string namet = fio::mktempfile(&ft);

        array_loop_t array_loop;
        array_loop.start(std::vector<std::string>(1, namet), arguments.size() == 2 ? arguments[1] : "");
        for (;;) // loop over all arrays in the CSV file
        {
            uintmax_t w = 0, h = 0;
            for (;;) // loop over all lines in the current CSV array
            {
                std::string line = fio::readline(fi, namei);
                if (std::ferror(fi))
                {
                    throw exc(namei + ": input error.");
                }
                if (std::feof(fi))
                {
                    break;
                }
                if (line.length() > 0 && line[line.length() - 1] == '\r')
                {
                    line.erase(line.length() - 1);
                }
                if (str::trim(line).empty())
                {
                    if (w > 0)
                    {
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }

                if (delim.empty())
                {
                    // Try to autodetect
                    const char* cline = line.c_str();
                    char* endptr;
                    (void)strtod(cline, &endptr);
                    if (endptr == cline || ((*endptr < 32 || *endptr >= 127) && *endptr != '\t'))
                    {
                        throw exc(namei + ": autodetection of delimiter failed; please specify with -D");
                    }
                    else
                    {
                        delim = std::string(1, *endptr);
                        std::string delimstr = (*endptr == '\t' ? "TAB"
                                : std::string(1, '\'') + delimiter.value() + std::string(1, '\''));
                        msg::inf(namei + ": autodetected delimiter is " + delimstr);
                    }
                }
                std::vector<std::string> value_strings = str::tokens(line, delim);
                if (w == 0)
                {
                    if (value_strings.size() == 0)
                    {
                        throw exc(namei + " array " + str::from(array_loop.index_out()) + " first row: no fields found.");
                    }
                    w = value_strings.size() / hdr.components();
                    if (value_strings.size() % hdr.components() != 0)
                    w++;
                    msg::inf(namei + " array " + str::from(array_loop.index_out()) + " first row: found " + str::from(w) + " field(s).");
                }

                blob element(checked_cast<size_t>(hdr.element_size()));
                size_t value_index = 0;
                for (uintmax_t e = 0; e < w; e++)
                {
                    // get next element
                    for (uintmax_t c = 0; c < hdr.components(); c++)
                    {
                        void *component = hdr.component(element.ptr(), c);
                        bool have_value = false;
                        if (value_index < value_strings.size())
                        {
                            have_value = parse_component(value_strings[value_index], hdr.component_type(c), component);
                            value_index++;
                        }
                        if (!have_value)
                        {
                            std::memcpy(component, hdr.component(no_data_element.ptr(), c), hdr.component_size(c));
                            msg::wrn(namei + " array " + str::from(array_loop.index_out()) + " row " + str::from(h) + " element " + str::from(e)
                                    + " component " + str::from(c) + ": no data available");
                        }
                    }
                    // write it to temporary file
                    fio::write(element.ptr(), hdr.element_size(), 1, ft, namet);
                }
                h++;
            }
            if (w == 0 || h == 0)
            {
                throw exc(namei + " array " + str::from(array_loop.index_out()) + " contains no data");
            }
            hdr.set_dimensions(w, h);
            // write gta: first write header, then copy data from temporary file
            array_loop.write(hdr, nameo);
            fio::flush(ft, namet); // make sure all data reached the temporary file
            array_loop.copy_data(hdr, hdr);
            // reuse the same space of the temporary file for the next array
            fio::rewind(ft, namet);
            fio::rewind(array_loop.file_in(), namet);
            if (std::feof(fi))
            {
                break;
            }
        }
        array_loop.finish();
        fio::close(fi, namei);
        fio::close(ft);
        fio::remove(namet);
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
