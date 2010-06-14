/*
 * to-mat.cpp
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
        // Open the file for writing to 1) truncate it if it exists and 2) give a
        // better error message if opening fails
        cio::close(cio::open(ofilename, "w"), ofilename);
        // Now open the output file with matio
        mat_t *mat = Mat_Open(ofilename.c_str(), MAT_ACC_RDWR);
        if (!mat)
        {
            throw exc("cannot open " + ofilename);
        }

        uintmax_t array_index = 0;
        while (cio::has_more(fi, ifilename))
        {
            std::string array_name = ifilename + " array " + str::from(array_index);
            gta::header hdr;
            hdr.read_from(fi);
            if (hdr.components() != 1)
            {
                throw exc("cannot export " + array_name, "only arrays with a single array element component can be exported to MATLAB");
            }
            if (hdr.dimensions() == 0)
            {
                msg::wrn(array_name + ": ignoring empty array");
                continue;
            }
            int class_type;
            int data_type;
            bool is_complex = false;
            switch (hdr.component_type(0))
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
                throw exc("cannot export " + ifilename,
                        std::string("data type ")
                        + type_to_string(hdr.component_type(0), hdr.component_size(0))
                        + " cannot be exported to MATLAB");
                break;
            }
            const char *name = hdr.global_taglist().get("MATLAB/NAME");
            int rank = checked_cast<int>(hdr.dimensions());
            if (rank < 2)
            {
                rank = 2;
            }
            std::vector<int> dims(rank);
            for (uintmax_t i = 0; i < hdr.dimensions(); i++)
            {
                dims[i] = checked_cast<int>(hdr.dimension_size(i));
            }
            if (hdr.dimensions() < 2)
            {
                dims[1] = 1;
            }
            int opt = MEM_CONSERVE;
            if (is_complex)
            {
                opt |= MAT_F_COMPLEX;
            }
            blob gta_data(checked_cast<size_t>(hdr.data_size()));
            blob mat_data;
            hdr.read_data(fi, gta_data.ptr());
            void *data = gta_data.ptr();
            struct ComplexSplit split_data;
            if (is_complex)
            {
                mat_data.resize(checked_cast<size_t>(hdr.data_size()));
                for (uintmax_t i = 0; i < hdr.elements(); i++)
                {
                    if (data_type == MAT_T_SINGLE)
                    {
                        float re = gta_data.ptr<float>()[2 * i + 0];
                        float im = gta_data.ptr<float>()[2 * i + 1];
                        mat_data.ptr<float>()[i] = re;
                        mat_data.ptr<float>()[hdr.elements() + i] = im;
                    }
                    else
                    {
                        double re = gta_data.ptr<double>()[2 * i + 0];
                        double im = gta_data.ptr<double>()[2 * i + 1];
                        mat_data.ptr<double>()[i] = re;
                        mat_data.ptr<double>()[hdr.elements() + i] = im;
                    }
                }
                split_data.Re = mat_data.ptr();
                if (data_type == MAT_T_SINGLE)
                {
                    split_data.Im = mat_data.ptr<float>(hdr.elements());
                }
                else
                {
                    split_data.Im = mat_data.ptr<double>(hdr.elements());
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
