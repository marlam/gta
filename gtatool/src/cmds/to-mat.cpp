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

#include <string>
#include <limits>

#include <matio.h>

#include <gta/gta.hpp>

#include "msg.h"
#include "blob.h"
#include "cio.h"
#include "opt.h"
#include "debug.h"
#include "intcheck.h"

#include "lib.h"


extern "C" void gtatool_to_mat_help(void)
{
    msg::req_txt("to-mat [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to the MATLB .mat format using matio.");
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

extern "C" int gtatool_to_mat(int argc, char *argv[])
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
        gtatool_to_mat_help();
        return 0;
    }

    FILE *fi = gtatool_stdin;
    std::string ifilename("standard input");
    std::string ofilename(arguments[0]);
    try
    {
        if (arguments.size() == 2)
        {
            ifilename = arguments[0];
            fi = cio::open(ifilename, "r");
            ofilename = arguments[1];
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    try
    {
        // Remove the output file if it exists so that matio does not append to it
        try { cio::remove(ofilename); } catch (...) {}
        mat_t *mat = Mat_Open(ofilename.c_str(), MAT_ACC_RDWR);
        if (!mat)
        {
            throw exc("cannot open " + ofilename);
        }

        uintmax_t array_index = 0;
        while (cio::has_more(fi, ifilename))
        {
            std::string array_name = ifilename + " array " + str::from(array_index);
            gta::header ihdr;
            ihdr.read_from(fi);
            if (ihdr.components() != 1)
            {
                throw exc("cannot export " + array_name + ": only arrays with a single array element component can be exported to MATLAB");
            }
            if (ihdr.dimensions() == 0)
            {
                msg::wrn(array_name + ": ignoring empty array");
                continue;
            }
            int class_type;
            int data_type;
            bool is_complex = false;
            switch (ihdr.component_type(0))
            {
            case gta::int8:
                class_type = MAT_C_INT8;
                data_type = MAT_T_INT8;
                break;
            case gta::uint8:
                class_type = MAT_C_UINT8;
                data_type = MAT_T_UINT8;
                break;
            case gta::int16:
                class_type = MAT_C_INT16;
                data_type = MAT_T_INT16;
                break;
            case gta::uint16:
                class_type = MAT_C_UINT16;
                data_type = MAT_T_UINT16;
                break;
            case gta::int32:
                class_type = MAT_C_INT32;
                data_type = MAT_T_INT32;
                break;
            case gta::uint32:
                class_type = MAT_C_UINT32;
                data_type = MAT_T_UINT32;
                break;
            case gta::int64:
                class_type = MAT_C_INT64;
                data_type = MAT_T_INT64;
                break;
            case gta::uint64:
                class_type = MAT_C_UINT64;
                data_type = MAT_T_UINT64;
                break;
            case gta::float32:
                class_type = MAT_C_SINGLE;
                data_type = MAT_T_SINGLE;
                break;
            case gta::float64:
                class_type = MAT_C_DOUBLE;
                data_type = MAT_T_DOUBLE;
                break;
            case gta::cfloat32:
                class_type = MAT_C_SINGLE;
                data_type = MAT_T_SINGLE;
                is_complex = true;
                break;
            case gta::cfloat64:
                class_type = MAT_C_DOUBLE;
                data_type = MAT_T_DOUBLE;
                is_complex = true;
                break;
            default:
                throw exc("cannot export " + ifilename + ": data type "
                        + type_to_string(ihdr.component_type(0), ihdr.component_size(0))
                        + " cannot be exported to MATLAB");
                break;
            }
            blob idata(checked_cast<size_t>(ihdr.data_size()));
            ihdr.read_data(fi, idata.ptr());
            gta::header ohdr;
            blob odata(checked_cast<size_t>(ihdr.data_size()));
            reorder_matlab_data(ohdr, odata.ptr(), ihdr, idata.ptr());
            idata.resize(0);
            const char *name = ohdr.global_taglist().get("MATLAB/NAME");
            int rank = checked_cast<int>(ohdr.dimensions());
            if (rank < 2)
            {
                rank = 2;
            }
            std::vector<int> dims(rank);
            for (uintmax_t i = 0; i < ohdr.dimensions(); i++)
            {
                dims[i] = checked_cast<int>(ohdr.dimension_size(i));
            }
            if (ohdr.dimensions() < 2)
            {
                dims[1] = 1;
            }
            int opt = MEM_CONSERVE;
            if (is_complex)
            {
                opt |= MAT_F_COMPLEX;
            }
            void *data = odata.ptr();
            blob tmp_data;
            struct ComplexSplit split_data;
            if (is_complex)
            {
                tmp_data.resize(checked_cast<size_t>(ohdr.data_size()));
                for (uintmax_t i = 0; i < ohdr.elements(); i++)
                {
                    if (data_type == MAT_T_SINGLE)
                    {
                        float re = odata.ptr<float>()[2 * i + 0];
                        float im = odata.ptr<float>()[2 * i + 1];
                        tmp_data.ptr<float>()[i] = re;
                        tmp_data.ptr<float>()[ohdr.elements() + i] = im;
                    }
                    else
                    {
                        double re = odata.ptr<double>()[2 * i + 0];
                        double im = odata.ptr<double>()[2 * i + 1];
                        tmp_data.ptr<double>()[i] = re;
                        tmp_data.ptr<double>()[ohdr.elements() + i] = im;
                    }
                }
                split_data.Re = tmp_data.ptr();
                if (data_type == MAT_T_SINGLE)
                {
                    split_data.Im = tmp_data.ptr<float>(ohdr.elements());
                }
                else
                {
                    split_data.Im = tmp_data.ptr<double>(ohdr.elements());
                }
                data = &split_data;
            }
            matvar_t *matvar = Mat_VarCreate(
                    name ? name : (std::string("gta_") + str::from(array_index)).c_str(),
                    class_type, data_type, rank, &(dims[0]), data, opt);
            if (!matvar)
            {
                throw exc("cannot create MATLAB variable");
            }
            if (Mat_VarWrite(mat, matvar, 0) != 0)
            {
                throw exc("cannot write MATLAB variable");
            }
            Mat_VarFree(matvar);
            array_index++;
        }
        if (fi != gtatool_stdin)
        {
            cio::close(fi);
        }
        Mat_Close(mat);
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
