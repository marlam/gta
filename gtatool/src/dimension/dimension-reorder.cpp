/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010, 2011, 2012, 2013
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

#include <gta/gta.hpp>

#include "base/msg.h"
#include "base/blb.h"
#include "base/opt.h"
#include "base/fio.h"
#include "base/str.h"
#include "base/chk.h"

#include "lib.h"


extern "C" void gtatool_dimension_reorder_help(void)
{
    msg::req_txt(
            "dimension-reorder [-i|--indices=<i0>[,<i1>[,...]]] [<files>...]\n"
            "\n"
            "Reorders the dimensions of the input GTAs into the given new order.\n"
            "The default is to make no changes.\n"
            "Example: dimension-reorder -i 1,0 matrix.gta > transposed.gta");
}

extern "C" int gtatool_dimension_reorder(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::tuple<uintmax_t> indices("indices", 'i', opt::optional);
    options.push_back(&indices);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_dimension_reorder_help();
        return 0;
    }
    for (size_t i = 0; i < indices.value().size(); i++)
    {
        if (indices.value()[i] >= indices.value().size())
        {
            msg::err_txt("invalid index %s in list of %s indices",
                    str::from(indices.value()[i]).c_str(),
                    str::from(indices.value().size()).c_str());
            return 1;
        }
        for (size_t j = 0; j < i; j++)
        {
            if (indices.value()[i] == indices.value()[j])
            {
                msg::err_txt("index %s was used more than once",
                        str::from(indices.value()[i]).c_str());
                return 1;
            }
        }
    }

    try
    {
        array_loop_t array_loop;
        gta::header hdri, hdro;
        std::string namei, nameo;
        array_loop.start(arguments, "");
        while (array_loop.read(hdri, namei))
        {
            if (!indices.value().empty() && hdri.dimensions() != indices.value().size())
            {
                throw exc(namei + ": array has " + str::from(hdri.dimensions())
                        + " dimensions while list of indices has " + str::from(indices.value().size()));
            }
            uintmax_t data_offset = 0;
            FILE *fbuf = NULL;
            gta::header hbuf;
            if (!fio::seekable(array_loop.file_in()) || hdri.compression() != gta::none)
            {
                buffer_data(hdri, array_loop.file_in(), hbuf, &fbuf);
            }
            else
            {
                data_offset = fio::tell(array_loop.file_in(), array_loop.filename_in());
            }
            hdro = hdri;
            hdro.set_compression(gta::none);
            if (!indices.value().empty())
            {
                std::vector<uintmax_t> dim_sizes;
                for (size_t i = 0; i < indices.value().size(); i++)
                {
                    dim_sizes.push_back(hdri.dimension_size(indices.value()[i]));
                }
                hdro.set_dimensions(dim_sizes.size(), &(dim_sizes[0]));
                for (size_t i = 0; i < indices.value().size(); i++)
                {
                    hdro.dimension_taglist(i) = hdri.dimension_taglist(indices.value()[i]);
                }
            }
            array_loop.write(hdro, nameo);
            element_loop_t element_loop;
            array_loop.start_element_loop(element_loop, hdri, hdro);
            blob element(checked_cast<size_t>(hdri.element_size()));
            std::vector<uintmax_t> in_indices(hdri.dimensions());
            std::vector<uintmax_t> out_indices(hdro.dimensions());
            for (uintmax_t e = 0; e < hdro.elements(); e++)
            {
                hdro.linear_index_to_indices(e, &(out_indices[0]));
                for (size_t i = 0; i < out_indices.size(); i++)
                {
                    if (!indices.value().empty())
                    {
                        in_indices[indices.value()[i]] = out_indices[i];
                    }
                    else
                    {
                        in_indices[i] = out_indices[i];
                    }
                }
                if (fbuf)
                {
                    hbuf.read_block(fbuf, 0, &(in_indices[0]), &(in_indices[0]), element.ptr());
                }
                else
                {
                    hdri.read_block(array_loop.file_in(), data_offset, &(in_indices[0]), &(in_indices[0]), element.ptr());
                }
                element_loop.write(element.ptr());
            }
            if (fbuf)
            {
                fclose(fbuf);
            }
            else
            {
                fio::seek(array_loop.file_in(), data_offset, SEEK_SET, array_loop.filename_in());
                array_loop.skip_data(hdri);
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
