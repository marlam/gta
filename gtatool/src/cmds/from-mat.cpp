/*
 * from-mat.cpp
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

#include <string>

#include <matio.h>

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "cio.h"
#include "opt.h"
#include "debug.h"
#include "intcheck.h"

#include "lib.h"


extern "C" void gtatool_from_mat_help(void)
{
    msg::req_txt("from-mat <input-file> [<output-file>]\n"
            "\n"
            "Converts MATLAB .mat files to GTAs using matio.");
}

static void reorder_matlab_data(gta::header &dsthdr, void *dst, const gta::header &srchdr, const void *src)
{
    dsthdr = srchdr;
    std::vector<uintmax_t> dstdims(checked_cast<size_t>(srchdr.dimensions()));
    for (size_t i = 0; i < dstdims.size(); i++)
    {
        dstdims[i] = srchdr.dimension_size(dstdims.size() - 1 - i);
    }
    dsthdr.set_dimensions(dstdims.size(), &(dstdims[0]));
    std::vector<uintmax_t> dstindices(checked_cast<size_t>(dsthdr.dimensions()));
    std::vector<uintmax_t> srcindices(checked_cast<size_t>(srchdr.dimensions()));
    for (uintmax_t i = 0; i < dsthdr.elements(); i++)
    {
        dsthdr.linear_index_to_indices(i, &(dstindices[0]));
        for (uintmax_t j = 0; j < dsthdr.dimensions(); j++)
        {
            srcindices[j] = dstindices[dsthdr.dimensions() - 1 - j];
        }
        uintmax_t k = srchdr.indices_to_linear_index(&(srcindices[0]));
        memcpy(dsthdr.element(dst, i), srchdr.element(src, k), dsthdr.element_size());
    }
}

extern "C" int gtatool_from_mat(int argc, char *argv[])
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
        gtatool_from_mat_help();
        return 0;
    }

    FILE *fo = gtatool_stdout;
    std::string ifilename(arguments[0]);
    std::string ofilename("standard output");
    try
    {
        if (arguments.size() == 2)
        {
            ofilename = arguments[1];
            fo = cio::open(ofilename, "w");
        }
        if (cio::isatty(fo))
        {
            throw exc("refusing to write to a tty");
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    try
    {
        mat_t *mat = Mat_Open(ifilename.c_str(), MAT_ACC_RDONLY);
        if (!mat)
        {
            throw exc("cannot open " + ifilename);
        }

        matvar_t *matvar;
        while ((matvar = Mat_VarReadNext(mat)))
        {
            gta::header ihdr;
            std::vector<uintmax_t> dimensions(checked_cast<size_t>(matvar->rank));
            for (size_t i = 0; i < dimensions.size(); i++)
            {
                dimensions[i] = checked_cast<uintmax_t>(matvar->dims[i]);
                if (matvar->dims[i] < 1)
                {
                    throw exc(ifilename, ": MATLAB data has invalid dimensions");
                }
            }
            ihdr.set_dimensions(dimensions.size(), &(dimensions[0]));
            gta::type type;
            if (matvar->data_type == MAT_T_INT8 && !matvar->isComplex)
            {
                type = gta::int8;
            }
            else if (matvar->data_type == MAT_T_UINT8 && !matvar->isComplex)
            {
                type = gta::uint8;
            }
            else if (matvar->data_type == MAT_T_INT16 && !matvar->isComplex)
            {
                type = gta::int16;
            }
            else if (matvar->data_type == MAT_T_UINT16 && !matvar->isComplex)
            {
                type = gta::uint16;
            }
            else if (matvar->data_type == MAT_T_INT32 && !matvar->isComplex)
            {
                type = gta::int32;
            }
            else if (matvar->data_type == MAT_T_UINT32 && !matvar->isComplex)
            {
                type = gta::uint32;
            }
            else if (matvar->data_type == MAT_T_INT64 && !matvar->isComplex)
            {
                type = gta::int64;
            }
            else if (matvar->data_type == MAT_T_UINT64 && !matvar->isComplex)
            {
                type = gta::uint64;
            }
            else if (matvar->data_type == MAT_T_SINGLE)
            {
                type = (matvar->isComplex ? gta::cfloat32 : gta::float32);
            }
            else if (matvar->data_type == MAT_T_DOUBLE)
            {
                type = (matvar->isComplex ? gta::cfloat64 : gta::float64);
            }
            else
            {
                std::string type_name =
                    (  matvar->data_type == MAT_T_INT8 ? "INT8"
                     : matvar->data_type == MAT_T_UINT8 ? "UINT8"
                     : matvar->data_type == MAT_T_INT16 ? "INT16"
                     : matvar->data_type == MAT_T_UINT16 ? "UINT16"
                     : matvar->data_type == MAT_T_INT32 ? "INT32"
                     : matvar->data_type == MAT_T_UINT32 ? "UINT32"
                     : matvar->data_type == MAT_T_INT64 ? "INT64"
                     : matvar->data_type == MAT_T_UINT64 ? "UINT64"
                     : matvar->data_type == MAT_T_SINGLE ? "SINGLE"
                     : matvar->data_type == MAT_T_DOUBLE ? "DOUBLE"
                     : matvar->data_type == MAT_T_MATRIX ? "MATRIX"
                     : matvar->data_type == MAT_T_COMPRESSED ? "COMPRESSED"
                     : matvar->data_type == MAT_T_UTF8 ? "UTF8"
                     : matvar->data_type == MAT_T_UTF16 ? "UTF16"
                     : matvar->data_type == MAT_T_UTF32 ? "UTF32"
                     : matvar->data_type == MAT_T_STRING ? "STRING"
                     : matvar->data_type == MAT_T_CELL ? "CELL"
                     : matvar->data_type == MAT_T_STRUCT ? "STRUCT"
                     : matvar->data_type == MAT_T_ARRAY ? "ARRAY"
                     : matvar->data_type == MAT_T_FUNCTION ? "FUNCTION"
                     : matvar->data_type == MAT_T_UNKNOWN ? "UNKNOWN"
                     : "(unknown)");
                msg::wrn("ignoring variable of type " + type_name + (matvar->isComplex ? " (complex)" : ""));
                continue;
            }
            ihdr.set_components(type);
            if (matvar->name && matvar->name[0] != '\0')
            {
                ihdr.global_taglist().set("MATLAB/NAME", matvar->name);
            }
            gta::header ohdr;
            blob output_data(checked_cast<size_t>(ihdr.data_size()));
            if (matvar->isComplex)
            {
                const struct ComplexSplit *split_data = static_cast<const struct ComplexSplit *>(matvar->data);
                blob fixed_data(checked_cast<size_t>(ihdr.data_size()));
                for (uintmax_t i = 0; i < ihdr.elements(); i++)
                {
                    if (type == gta::cfloat32)
                    {
                        float re = (static_cast<const float *>(split_data->Re))[i];
                        float im = (static_cast<const float *>(split_data->Im))[i];
                        fixed_data.ptr<float>()[2 * i + 0] = re;
                        fixed_data.ptr<float>()[2 * i + 1] = im;
                    }
                    else
                    {
                        double re = (static_cast<const double *>(split_data->Re))[i];
                        double im = (static_cast<const double *>(split_data->Im))[i];
                        fixed_data.ptr<double>()[2 * i + 0] = re;
                        fixed_data.ptr<double>()[2 * i + 1] = im;
                    }
                }
                reorder_matlab_data(ohdr, output_data.ptr(), ihdr, fixed_data.ptr());
            }
            else
            {
                reorder_matlab_data(ohdr, output_data.ptr(), ihdr, matvar->data);
            }
            ohdr.write_to(fo);
            ohdr.write_data(fo, output_data.ptr());
            Mat_VarFree(matvar);
        }
        if (fo != gtatool_stdout)
        {
            cio::close(fo);
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
