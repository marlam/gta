/*
 * stream-split.cpp
 *
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
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

#include <gta/gta.hpp>

#include "msg.h"
#include "opt.h"
#include "cio.h"
#include "str.h"

#include "lib.h"


extern "C" void gtatool_stream_split_help(void)
{
    msg::req_txt(
            "stream-split [<template>] [<files...>]\n"
            "\n"
            "Writes the input arrays into separate files, using a file name template.\n"
            "The template must contain the sequence %%[n]N, which will be replaced by the "
            "index of the array in the input stream. The optional parameter n gives the minimum "
            "number of digits in the index number; small indices will be padded with zeroes. "
            "The default template is %%9N.gta.\n"
            "Example:\n"
            "stream-split array-%%3N.gta 129-arrays.gta");
}

extern "C" int gtatool_stream_split(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_stream_split_help();
        return 0;
    }

    try
    {
        std::string tmpl;
        if (arguments.size() == 0)
        {
            tmpl = "%9N.gta";
        }
        else
        {
            tmpl = arguments.front();
            arguments.erase(arguments.begin());
        }
        size_t seq_start = tmpl.find_first_of('%');
        if (seq_start == std::string::npos)
        {
            throw exc("the template argument does not contain the sequence %[n]N");
        }
        size_t seq_end = tmpl.find_first_of('N', seq_start);
        if (seq_end == std::string::npos)
        {
            throw exc("the template argument does not contain the sequence %[n]N");
        }
        size_t seq_length = seq_end - seq_start + 1;
        unsigned int min_width;
        if (seq_length == 2)
        {
            min_width = 0;
        }
        else
        {        
            try
            {
                min_width = str::to<unsigned int>(tmpl.substr(seq_start + 1, seq_length - 2));
            }
            catch (...)
            {
                throw exc("the template argument does not contain the sequence %[n]N");
            }
        }

        array_loop_t array_loop;
        gta::header hdri, hdro;
        std::string namei, nameo;
        array_loop.start(arguments, "");
        uintmax_t array_index = 0;
        while (array_loop.read(hdri, namei))
        {
            // Open the output file
            std::string array_index_str = str::from(array_index);
            if (array_index_str.length() < min_width)
            {
                array_index_str.insert(0, min_width - array_index_str.length(), '0');
            }
            std::string foname = tmpl;
            foname.replace(seq_start, seq_length, array_index_str);
            FILE *fo = cio::open(foname, "w");                
            // Write the GTA header
            hdro = hdri;
            hdro.set_compression(gta::none);
            hdro.write_to(fo);
            // Copy the GTA data
            hdri.copy_data(array_loop.file_in(), hdro, fo);
            cio::close(fo, foname);
            array_index++;
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
