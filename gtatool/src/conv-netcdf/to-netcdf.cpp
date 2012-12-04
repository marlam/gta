/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2012
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

#include <gta/gta.hpp>

#include <netcdf.h>

#include "msg.h"
#include "str.h"
#include "exc.h"
#include "fio.h"
#include "opt.h"
#include "intcheck.h"
#include "blob.h"

#include "lib.h"


extern "C" void gtatool_to_netcdf_help(void)
{
    msg::req_txt("to-netcdf [<input-file>] <output-file>\n"
            "\n"
            "Converts GTAs to the NetCDF file format (*.nc).\n"
            "You can create groups inside the NetCDF file by assigning NETCDF/GROUP=GROUPNAME tags "
            "to the global taglists of the GTAs. By default, only the single group \"/\" exists.\n"
            "The first GTA in a group defines the dimensions for all following variables in the same group.");
}

extern "C" int gtatool_to_netcdf(int argc, char *argv[])
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
        gtatool_to_netcdf_help();
        return 0;
    }

    try
    {
        std::string nameo = arguments.size() == 1 ? arguments[0] : arguments[1];
        array_loop_t array_loop;
        gta::header hdr;
        std::string name;
        const char* tagval;

        int nc_err;
        int nc_file;

        nc_err = nc_create(nameo.c_str(), NC_CLOBBER | NC_NETCDF4, &nc_file);
        if (nc_err != 0)
            throw exc(nameo + ": " + nc_strerror(nc_err));

        array_loop.start(arguments.size() == 1 ? std::vector<std::string>() : std::vector<std::string>(1, arguments[0]), nameo);
        while (array_loop.read(hdr, name))
        {
            if (hdr.dimensions() == 0 || hdr.components() == 0)
            {
                msg::wrn(name + ": skipping empty array - meta information might be lost");
                continue;
            }
            if (hdr.components() > 1)
                throw exc(name + ": cannot handle arrays with more than one element component");

            /* Determine the NetCDF data type */
            nc_type nc_xtype;
            switch (hdr.component_type(0))
            {
            case gta::int8:
                nc_xtype = NC_BYTE;
                break;
            case gta::uint8:
                nc_xtype = NC_UBYTE;
                break;
            case gta::int16:
                nc_xtype = NC_SHORT;
                break;
            case gta::uint16:
                nc_xtype = NC_USHORT;
                break;
            case gta::int32:
                nc_xtype = NC_INT;
                break;
            case gta::uint32:
                nc_xtype = NC_UINT;
                break;
            case gta::int64:
                nc_xtype = NC_INT64;
                break;
            case gta::uint64:
                nc_xtype = NC_UINT64;
                break;
            case gta::float32:
                nc_xtype = NC_FLOAT;
                break;
            case gta::float64:
                nc_xtype = NC_DOUBLE;
                break;
            default:
                throw exc(name + ": element component type "
                        + type_to_string(hdr.component_type(0), hdr.component_size(0))
                        + " not supported by NetCDF");
                break;
            }

            /* Put the GTA in the correct NetCDF group */
            std::string nc_group;
            tagval = hdr.global_taglist().get("NETCDF/GROUP");
            if (tagval && !(tagval[0] == '/' && tagval[1] == '\0'))
                nc_group = tagval;
            int nc_group_id = nc_file;
            if (!nc_group.empty())
            {
                nc_err = nc_inq_ncid(nc_file, nc_group.c_str(), &nc_group_id);
                if (nc_err != 0)
                {
                    /* a group with this name does not exist yet, so create it */
                    nc_err = nc_def_grp(nc_file, nc_group.c_str(), &nc_group_id);
                    if (nc_err != 0)
                        throw exc(nameo + ": " + nc_strerror(nc_err));
                }
            }

            /* Handle dimensions. */
            int nc_dimensions;
            nc_err = nc_inq_ndims(nc_group_id, &nc_dimensions);
            if (nc_err != 0)
                throw exc(nameo + ": " + nc_strerror(nc_err));
            if (nc_dimensions == 0)
            {
                /* Define dimensions. This is only done once per group, by the first GTA in the group. */
                for (uintmax_t dd = 0; dd < hdr.dimensions(); dd++)
                {
                    uintmax_t d = hdr.dimensions() - dd - 1;
                    std::string nc_dim_name;
                    if ((tagval = hdr.dimension_taglist(d).get("NETCDF/NAME")))
                        nc_dim_name = tagval;
                    if (nc_dim_name.empty() && (tagval = hdr.dimension_taglist(d).get("INTERPRETATION")))
                        nc_dim_name = tagval;
                    if (nc_dim_name.empty())
                        nc_dim_name = std::string("DIM") + str::from(d);
                    int nc_dim_id;
                    nc_err = nc_def_dim(nc_group_id, nc_dim_name.c_str(), checked_cast<size_t>(hdr.dimension_size(d)), &nc_dim_id);
                    if (nc_err != 0)
                        throw exc(nameo + ": " + nc_strerror(nc_err));
                    msg::dbg(name + ": GTA dimension " + str::from(d)
                            + " -> NetCDF dimension " + str::from(nc_dim_id) + " (\"" + nc_dim_name + "\")");
                }
                nc_dimensions = checked_cast<int>(hdr.dimensions());
            }
            else
            {
                /* Check that the dimensions match the dimensions of the current GTA. */
                if (hdr.dimensions() != checked_cast<uintmax_t>(nc_dimensions))
                    throw exc(name + ": dimensions differ from previous GTA in the same NetCDF group");
                for (uintmax_t dd = 0; dd < hdr.dimensions(); dd++)
                {
                    uintmax_t d = hdr.dimensions() - dd - 1;
                    size_t nc_dim_len;
                    nc_err = nc_inq_dimlen(nc_group_id, checked_cast<int>(d), &nc_dim_len);
                    if (nc_err != 0)
                        throw exc(nameo + ": " + nc_strerror(nc_err));
                    if (nc_dim_len != hdr.dimension_size(d))
                        throw exc(name + ": dimensions differ from previous GTA in the same NetCDF group");
                }
            }

            /* Create a variable for the current GTA */
            std::string nc_var_name;
            if ((tagval = hdr.global_taglist().get("NETCDF/NAME")))
                nc_var_name = tagval;
            if (nc_var_name.empty())
                nc_var_name = fio::basename(name);
            int nc_var_id;
            std::vector<int> nc_var_dim_ids(nc_dimensions);
            for (int i = 0; i < nc_dimensions; i++)
                nc_var_dim_ids[i] = i;
            nc_err = nc_def_var(nc_group_id, nc_var_name.c_str(), nc_xtype, nc_dimensions, &(nc_var_dim_ids[0]), &nc_var_id);
            if (nc_err != 0)
                throw exc(nameo + ": " + nc_strerror(nc_err));

            /* Assign attributes to the variable */
            for (uintmax_t t = 0; t < hdr.global_taglist().tags(); t++)
            {
                std::string nc_att_name = hdr.global_taglist().name(t);
                std::string nc_att_value = hdr.global_taglist().value(t);
                if (nc_att_name == std::string("NETCDF/GROUP")
                        || nc_att_name == std::string("NETCDF/NAME")
                        || nc_att_name == std::string("NETCDF/_FillValue"))
                    continue;
                if (nc_att_name.substr(0, 7) == std::string("NETCDF/"))
                    nc_att_name = nc_att_name.substr(7);
                nc_err = nc_put_att_text(nc_group_id, nc_var_id, nc_att_name.c_str(), nc_att_value.length(), nc_att_value.c_str());
                if (nc_err != 0)
                    throw exc(nameo + ": " + nc_strerror(nc_err));
            }
            for (uintmax_t d = 0; d < hdr.dimensions(); d++)
            {
                for (uintmax_t t = 0; t < hdr.dimension_taglist(d).tags(); t++)
                {
                    std::string nc_att_name = hdr.dimension_taglist(d).name(t);
                    std::string nc_att_value = hdr.dimension_taglist(d).value(t);
                    if (nc_att_name == std::string("NETCDF/NAME"))
                        continue;
                    else
                        nc_att_name = std::string("DIM") + str::from(d) + std::string("/") + nc_att_name;
                    nc_err = nc_put_att_text(nc_group_id, nc_var_id, nc_att_name.c_str(), nc_att_value.length(), nc_att_value.c_str());
                    if (nc_err != 0)
                        throw exc(nameo + ": " + nc_strerror(nc_err));
                }
            }
            for (uintmax_t t = 0; t < hdr.component_taglist(0).tags(); t++)
            {
                std::string nc_att_name = hdr.component_taglist(0).name(t);
                std::string nc_att_value = hdr.component_taglist(0).value(t);
                if (nc_att_name == std::string("NO_DATA_VALUE"))
                {
                    if (nc_xtype == NC_BYTE)
                    {
                        signed char x = str::to<signed char>(nc_att_value);
                        nc_err = nc_put_att_schar(nc_group_id, nc_var_id, "_FillValue", nc_xtype, 1, &x);
                    }
                    else if (nc_xtype == NC_UBYTE)
                    {
                        unsigned char x = str::to<unsigned char>(nc_att_value);
                        nc_err = nc_put_att_uchar(nc_group_id, nc_var_id, "_FillValue", nc_xtype, 1, &x);
                    }
                    else if (nc_xtype == NC_SHORT)
                    {
                        short x = str::to<short>(nc_att_value);
                        nc_err = nc_put_att_short(nc_group_id, nc_var_id, "_FillValue", nc_xtype, 1, &x);
                    }
                    else if (nc_xtype == NC_USHORT)
                    {
                        unsigned short x = str::to<unsigned short>(nc_att_value);
                        nc_err = nc_put_att_ushort(nc_group_id, nc_var_id, "_FillValue", nc_xtype, 1, &x);
                    }
                    else if (nc_xtype == NC_INT)
                    {
                        int x = str::to<int>(nc_att_value);
                        nc_err = nc_put_att_int(nc_group_id, nc_var_id, "_FillValue", nc_xtype, 1, &x);
                    }
                    else if (nc_xtype == NC_UINT)
                    {
                        unsigned int x = str::to<unsigned int>(nc_att_value);
                        nc_err = nc_put_att_uint(nc_group_id, nc_var_id, "_FillValue", nc_xtype, 1, &x);
                    }
                    else if (nc_xtype == NC_INT64)
                    {
                        long long x = str::to<long long>(nc_att_value);
                        nc_err = nc_put_att_longlong(nc_group_id, nc_var_id, "_FillValue", nc_xtype, 1, &x);
                    }
                    else if (nc_xtype == NC_UINT64)
                    {
                        unsigned long long x = str::to<unsigned long long>(nc_att_value);
                        nc_err = nc_put_att_ulonglong(nc_group_id, nc_var_id, "_FillValue", nc_xtype, 1, &x);
                    }
                    else if (nc_xtype == NC_FLOAT)
                    {
                        float x = str::to<float>(nc_att_value);
                        nc_err = nc_put_att_float(nc_group_id, nc_var_id, "_FillValue", nc_xtype, 1, &x);
                    }
                    else if (nc_xtype == NC_DOUBLE)
                    {
                        double x = str::to<double>(nc_att_value);
                        nc_err = nc_put_att_double(nc_group_id, nc_var_id, "_FillValue", nc_xtype, 1, &x);
                    }
                    if (nc_err != 0)
                        throw exc(nameo + ": " + nc_strerror(nc_err));
                }
                else if (nc_att_name == std::string("UNITS"))
                {
                    nc_att_name = "units";
                }
                else
                {
                    nc_att_name = std::string("COMPONENT/") + nc_att_name;
                }
                nc_err = nc_put_att_text(nc_group_id, nc_var_id, nc_att_name.c_str(), nc_att_value.length(), nc_att_value.c_str());
                if (nc_err != 0)
                    throw exc(nameo + ": " + nc_strerror(nc_err));
            }

            /* Write the variable data */
            size_t max_elements = 16384;
            // Copy multiple elements at once to reduce overhead
            std::vector<size_t> nc_dim_sizes(nc_dimensions);
            for (int d = 0; d < nc_dimensions; d++)
            {
                nc_err = nc_inq_dimlen(nc_group_id, d, &(nc_dim_sizes[d]));
                if (nc_err != 0)
                    throw exc(nameo + ": " + nc_strerror(nc_err));
            }
            element_loop_t element_loop;
            array_loop.start_element_loop(element_loop, hdr, hdr);
            blob buf(max_elements * hdr.element_size());
            uintmax_t e = 0;
            std::vector<size_t> nc_var_index(nc_dimensions, 0);
            std::vector<size_t> nc_var_count(nc_dimensions, 1);
            while (e < hdr.elements())
            {
                size_t elements = nc_dim_sizes[nc_dimensions - 1] - nc_var_index[nc_dimensions - 1];
                if (elements > max_elements)
                    elements = max_elements;
                const void* buf = element_loop.read(elements);
                nc_var_count[nc_dimensions - 1] = elements;
                nc_err = nc_put_vara(nc_group_id, nc_var_id, &(nc_var_index[0]), &(nc_var_count[0]), buf);
                if (nc_err != 0)
                    throw exc(nameo + ": " + nc_strerror(nc_err));
                e += elements;
                if (e < hdr.elements())
                {
                    nc_var_index[nc_dimensions - 1] += elements;
                    size_t inc_index = nc_dimensions - 1;
                    while (nc_var_index[inc_index] == nc_dim_sizes[inc_index])
                    {
                        nc_var_index[inc_index] = 0;
                        inc_index--;
                        nc_var_index[inc_index]++;
                    }
                }
            }
        }
        array_loop.finish();
        nc_err = nc_close(nc_file);
        if (nc_err != 0)
            throw exc(nameo + ": " + nc_strerror(nc_err));
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
