/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010, 2011, 2013
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

#include "msg.h"
#include "blob.h"
#include "opt.h"
#include "fio.h"
#include "str.h"
#include "intcheck.h"

#include "lib.h"


extern "C" void gtatool_dimension_split_help(void)
{
    msg::req_txt(
            "dimension-split [-d|--dimension=<d>] [<files>...]\n"
            "\n"
            "Reduces the dimensionality of the input GTAs by splitting them at the given dimension.\n"
            "For example, a 13x17x19 volume can be split at dimension 2 into 19 2D images of size 13x17, or "
            "at dimension 0 into 13 images of size 17x19.\n"
            "By default, the GTAs are split at the highest dimension.\n"
            "If you only want to extract specific indices of a dimension, use the dimension-extract command instead.\n"
            "All output arrays will be written into a single stream; if you want separate files, "
            "pipe this stream through the stream-split command.\n"
            "Example: dimension-split volume.gta > slices.gta");
}

extern "C" int gtatool_dimension_split(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::val<uintmax_t> dimension("dimension", 'd', opt::optional);
    options.push_back(&dimension);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_dimension_split_help();
        return 0;
    }

    std::string tempdir;
    try
    {
        array_loop_t array_loop;
        gta::header hdri, hdro;
        std::string namei, nameo;
        array_loop.start(arguments, "");
        while (array_loop.read(hdri, namei))
        {
            if (hdri.dimensions() == 0)
            {
                throw exc(namei + ": array has no dimensions");
            }
            uintmax_t dim = (dimension.values().empty() ? hdri.dimensions() - 1 : dimension.value());
            if (dim >= hdri.dimensions())
            {
                throw exc(namei + ": array has no dimension " + str::from(dim));
            }
            uintmax_t dim_size = hdri.dimension_size(dim);
            // Determine the new dimensions and create the new header
            std::vector<uintmax_t> dim_sizes;
            if (hdri.dimensions() == 1)
            {
                dim_sizes.push_back(1);
            }
            else
            {
                for (uintmax_t i = 0; i < hdri.dimensions(); i++)
                {
                    if (i != dim)
                    {
                        dim_sizes.push_back(hdri.dimension_size(i));
                    }
                }
            }
            gta::header hdro = hdri;
            hdro.set_compression(gta::none);
            hdro.set_dimensions(dim_sizes.size(), &(dim_sizes[0]));
            uintmax_t hdro_dim = 0;
            if (hdri.dimensions() == 1)
            {
                hdro.dimension_taglist(0) = hdri.dimension_taglist(0);
            }
            else
            {
                for (uintmax_t j = 0; j < hdri.dimensions(); j++)
                {
                    if (j != dim)
                    {
                        hdro.dimension_taglist(hdro_dim) = hdri.dimension_taglist(j);
                        hdro_dim++;
                    }
                }
            }
            if (hdri.element_size() > 0)
            {
                // Write the GTA data to temporary files "tempdir/index"
                tempdir = fio::mktempdir();
                FILE* tmpf = NULL;
                std::string tmpf_name;
                uintmax_t tmpf_index = 0;
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, hdri, hdro);
                blob element(checked_cast<size_t>(hdri.element_size()));
                std::vector<uintmax_t> indices(hdri.dimensions());
                for (uintmax_t i = 0; i < hdri.elements(); i++)
                {
                    const void *e = element_loop.read();
                    hdri.linear_index_to_indices(i, &(indices[0]));
                    uintmax_t j = indices[dim];
                    if (!tmpf || tmpf_index != j)
                    {
                        if (tmpf)
                        {
                            fio::close(tmpf, tmpf_name);
                        }
                        tmpf_name = tempdir + "/" + str::from(j);
                        tmpf = fio::open(tmpf_name, "a");
                        tmpf_index = j;
                    }
                    fio::write(e, element.size(), 1, tmpf, tmpf_name);
                }
                if (tmpf)
                {
                    fio::close(tmpf, tmpf_name);
                }
                // Combine the GTAs to a single output stream
                for (uintmax_t i = 0; i < dim_size; i++)
                {
                    std::string nameo;
                    array_loop_t tmploop;
                    tmpf_name = tempdir + "/" + str::from(i);
                    tmploop.start(tmpf_name, "");
                    tmploop.write(hdro, nameo);
                    tmploop.copy_data(hdro, hdro);
                    tmploop.finish();
                    fio::remove(tmpf_name);
                }
                fio::rmdir(tempdir);
            }
            else
            {
                for (uintmax_t i = 0; i < dim_size; i++)
                {
                    array_loop.write(hdro, nameo);
                }
            }
        }
        array_loop.finish();
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        if (tempdir.length() > 0 && fio::test_d(tempdir))
        {
            fio::rm_r(tempdir);
        }
        return 1;
    }

    return 0;
}
