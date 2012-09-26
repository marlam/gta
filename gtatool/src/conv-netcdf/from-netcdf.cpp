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

#include <vector>
#include <list>
#include <limits>

#include <netcdf.h>

#include <gta/gta.hpp>

#include "msg.h"
#include "str.h"
#include "exc.h"
#include "opt.h"
#include "intcheck.h"
#include "blob.h"
#include "fio.h"
#include "dbg.h"

#include "lib.h"


template<typename T>
int nc_attval_to_string(int nc_file, int nc_var_id, const char* nc_name, size_t nc_l, std::string& value)
{
    T* buf = new T[nc_l];
    int nc_err = nc_get_att(nc_file, nc_var_id, nc_name, buf);
    if (nc_err == 0)
    {
        for (size_t i = 0; i < nc_l; i++)
        {
            value += str::from(buf[i]);
            if (i < nc_l - 1)
                value += ' ';
        }
    }
    delete[] buf;
    return nc_err;
}

// Convert a NetCDF attribute to a name/value pair suitable for use as a GTA tag
int nc_att_to_tag(int nc_file, int nc_var_id, int nc_att_index, std::string& name, std::string& value)
{
    int nc_err;
    char nc_name[NC_MAX_NAME + 1];
    nc_type nc_t;
    size_t nc_l;

    nc_err = nc_inq_attname(nc_file, nc_var_id, nc_att_index, nc_name);
    if (nc_err != 0)
        return nc_err;
    nc_err = nc_inq_att(nc_file, nc_var_id, nc_name, &nc_t, &nc_l);
    if (nc_err != 0)
        return nc_err;

    name = std::string(nc_name);
    value.clear();
    if (nc_l == 0)
    {
        return 0;
    }
    else if (nc_t == NC_STRING || nc_t == NC_CHAR)
    {
        char* tmpstr = new char[nc_l + 1];
        nc_err = nc_get_att_text(nc_file, nc_var_id, nc_name, tmpstr);
        tmpstr[nc_l] = '\0';
        value = str::sanitize(std::string(tmpstr));
        delete[] tmpstr;
        if (nc_err != 0)
            return nc_err;
    }
    else if (nc_t == NC_BYTE)
    {
        nc_err = nc_attval_to_string<signed char>(nc_file, nc_var_id, nc_name, nc_l, value);
        if (nc_err != 0)
            return nc_err;
    }
    else if (nc_t == NC_SHORT)
    {
        nc_err = nc_attval_to_string<short>(nc_file, nc_var_id, nc_name, nc_l, value);
        if (nc_err != 0)
            return nc_err;
    }
    else if (nc_t == NC_INT)
    {
        nc_err = nc_attval_to_string<int>(nc_file, nc_var_id, nc_name, nc_l, value);
        if (nc_err != 0)
            return nc_err;
    }
    else if (nc_t == NC_FLOAT)
    {
        nc_err = nc_attval_to_string<float>(nc_file, nc_var_id, nc_name, nc_l, value);
        if (nc_err != 0)
            return nc_err;
    }
    else if (nc_t == NC_DOUBLE)
    {
        nc_err = nc_attval_to_string<double>(nc_file, nc_var_id, nc_name, nc_l, value);
        if (nc_err != 0)
            return nc_err;
    }
    else if (nc_t == NC_UBYTE)
    {
        nc_err = nc_attval_to_string<unsigned char>(nc_file, nc_var_id, nc_name, nc_l, value);
        if (nc_err != 0)
            return nc_err;
    }
    else if (nc_t == NC_USHORT)
    {
        nc_err = nc_attval_to_string<unsigned short>(nc_file, nc_var_id, nc_name, nc_l, value);
        if (nc_err != 0)
            return nc_err;
    }
    else if (nc_t == NC_UINT)
    {
        nc_err = nc_attval_to_string<unsigned int>(nc_file, nc_var_id, nc_name, nc_l, value);
        if (nc_err != 0)
            return nc_err;
    }
    else if (nc_t == NC_INT64)
    {
        nc_err = nc_attval_to_string<int64_t>(nc_file, nc_var_id, nc_name, nc_l, value);
        if (nc_err != 0)
            return nc_err;
    }
    else if (nc_t == NC_UINT64)
    {
        nc_err = nc_attval_to_string<uint64_t>(nc_file, nc_var_id, nc_name, nc_l, value);
        if (nc_err != 0)
            return nc_err;
    }
    else
    {
        return NC_EBADTYPE;
    }
    return 0;
}


extern "C" void gtatool_from_netcdf_help(void)
{
    msg::req_txt("from-netcdf <input-file> [<output-file>]\n"
            "\n"
            "Converts NetCDF files (*.nc) to GTAs.");
}

extern "C" int gtatool_from_netcdf(int argc, char *argv[])
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
        gtatool_from_netcdf_help();
        return 0;
    }

    try
    {
        array_loop_t array_loop;
        array_loop.start(std::vector<std::string>(1, arguments[0]), arguments.size() == 2 ? arguments[1] : "");

        std::string namei = arguments[0];
        int nc_err;
        int nc_file;
        char nc_name[NC_MAX_NAME + 1];

        nc_set_log_level(1);
        nc_err = nc_open(namei.c_str(), NC_NOWRITE, &nc_file);
        if (nc_err != 0)
            throw exc(namei + ": " + nc_strerror(nc_err));

        std::list<int> nc_groups(1, nc_file);
        while (!nc_groups.empty())
        {
            int nc_group = nc_groups.front();
            nc_groups.pop_front();
            nc_err = nc_inq_grpname(nc_group, nc_name);
            if (nc_err != 0)
                throw exc(namei + ": " + nc_strerror(nc_err));
            std::string nc_groupname(nc_name);

            int nc_dims, nc_vars, nc_atts, nc_unlimdim_id;
            nc_err = nc_inq(nc_group, &nc_dims, &nc_vars, &nc_atts, &nc_unlimdim_id);
            if (nc_err != 0)
                throw exc(namei + ": " + nc_strerror(nc_err));
            if (nc_dims < 0 || nc_vars < 0 || nc_atts < 0)
                throw exc(namei + ": invalid properties");

            /* First, create a dummy GTA that holds all global attributes.
             * These will be assigned to each individual variable. */
            gta::header global_hdr;
            global_hdr.global_taglist().set("NETCDF/GROUP", nc_groupname.c_str());
            for (int a = 0; a < nc_atts; a++)
            {
                std::string att_name, att_value;
                nc_err = nc_att_to_tag(nc_group, NC_GLOBAL, a, att_name, att_value);
                if (nc_err != 0)
                    throw exc(namei + ": " + nc_strerror(nc_err));
                try { global_hdr.global_taglist().set((std::string("NETCDF/") + att_name).c_str(), att_value.c_str()); }
                catch (...) { msg::wrn(namei + ": cannot set global attribute " + str::sanitize(att_name)); }
            }

            /* Convert all variables to separate GTAs. */
            for (int v = 0; v < nc_vars; v++)
            {
                nc_type nc_var_type;
                int nc_var_dims;
                std::vector<int> nc_var_dimids(nc_dims);
                int nc_var_atts;
                nc_err = nc_inq_var(nc_group, v, nc_name, &nc_var_type, &nc_var_dims,  &(nc_var_dimids[0]), &nc_var_atts);
                assert(nc_var_dims <= nc_dims);
                nc_var_dimids.resize(nc_var_dims);
                if (nc_err != 0)
                    throw exc(namei + ": " + nc_strerror(nc_err));

                gta::header hdr = global_hdr;       // Copy global taglist
                std::string nameo;
                gta::type type;
                hdr.global_taglist().set("NETCDF/NAME", nc_name);
                if (nc_var_dims > 0)
                {
                    std::vector<uintmax_t> dim_sizes(nc_var_dims);
                    std::vector<std::string> dim_names(nc_var_dims);
                    for (int d = 0; d < nc_var_dims; d++)
                    {
                        size_t nc_dim_size;
                        nc_err = nc_inq_dim(nc_group, nc_var_dimids[d], nc_name, &nc_dim_size);
                        if (nc_err != 0)
                            throw exc(namei + ": " + nc_strerror(nc_err));
                        if (nc_dim_size == 0)
                            throw exc(namei + ": cannot handle zero-size dimensions");
                        dim_sizes[nc_var_dims - d - 1] = nc_dim_size;
                        dim_names[nc_var_dims - d - 1] = std::string(nc_name);
                    }
                    hdr.set_dimensions(dim_sizes.size(), &(dim_sizes[0]));
                    for (size_t d = 0; d < dim_names.size(); d++)
                        hdr.dimension_taglist(d).set("NETCDF/NAME", dim_names[d].c_str());
                }
                for (int a = 0; a < nc_var_atts; a++)
                {
                    std::string att_name, att_value;
                    nc_err = nc_att_to_tag(nc_group, v, a, att_name, att_value);
                    if (nc_err != 0)
                        throw exc(namei + ": " + nc_strerror(nc_err));
                    try { hdr.global_taglist().set((std::string("NETCDF/") + att_name).c_str(), att_value.c_str()); }
                    catch (...) { msg::wrn(namei + ": cannot set variable attribute " + str::sanitize(att_name)); }
                }
                switch (nc_var_type)
                {
                case NC_BYTE:
                    type = gta::int8;
                    break;
                case NC_CHAR:
                    if (std::numeric_limits<char>::min() == 0)
                        type = gta::uint8;
                    else
                        type = gta::int8;
                    break;
                case NC_SHORT:
                    type = gta::int16;
                    break;
                case NC_INT:
                    type = gta::int32;
                    break;
                case NC_FLOAT:
                    type = gta::float32;
                    break;
                case NC_DOUBLE:
                    type = gta::float64;
                    break;
                case NC_UBYTE:
                    type = gta::uint8;
                    break;
                case NC_USHORT:
                    type = gta::uint16;
                    break;
                case NC_UINT:
                    type = gta::uint32;
                    break;
                case NC_INT64:
                    type = gta::int64;
                    break;
                case NC_UINT64:
                    type = gta::uint64;
                    break;
                default:
                    throw exc(namei + ": cannot handle variable type " + str::from(nc_var_type));
                }
                hdr.set_components(type);
                const char* tagval;
                if ((tagval = hdr.global_taglist().get("NETCDF/_FillValue"))
                        || (tagval = hdr.global_taglist().get("NETCDF/missing_value")))
                    hdr.component_taglist(0).set("NO_DATA_VALUE", tagval);
                if ((tagval = hdr.global_taglist().get("NETCDF/units")))
                    hdr.component_taglist(0).set("UNITS", tagval);
                array_loop.write(hdr, nameo);
                if (hdr.data_size() > 0)
                {
                    size_t max_elements = 16384;
                    // Copy multiple elements at once to reduce overhead
                    std::vector<size_t> nc_dim_sizes(nc_var_dims);
                    for (int d = 0; d < nc_var_dims; d++)
                    {
                        nc_err = nc_inq_dimlen(nc_group, nc_var_dimids[d], &(nc_dim_sizes[d]));
                        if (nc_err != 0)
                            throw exc(namei + ": " + nc_strerror(nc_err));
                    }
                    element_loop_t element_loop;
                    array_loop.start_element_loop(element_loop, hdr, hdr);
                    blob buf(max_elements * hdr.element_size());
                    uintmax_t e = 0;
                    std::vector<size_t> nc_var_index(nc_var_dims, 0);
                    std::vector<size_t> nc_var_count(nc_var_dims, 1);
                    while (e < hdr.elements())
                    {
                        size_t elements = nc_dim_sizes[nc_var_dims - 1] - nc_var_index[nc_var_dims - 1];
                        if (elements > max_elements)
                            elements = max_elements;
                        nc_var_count[nc_var_dims - 1] = elements;
                        nc_err = nc_get_vara(nc_group, v, &(nc_var_index[0]), &(nc_var_count[0]), buf.ptr());
                        if (nc_err != 0)
                            throw exc(namei + ": " + nc_strerror(nc_err));
                        element_loop.write(buf.ptr(), elements);
                        e += elements;
                        if (e < hdr.elements())
                        {
                            nc_var_index[nc_var_dims - 1] += elements;
                            size_t inc_index = nc_var_dims - 1;
                            while (nc_var_index[inc_index] == nc_dim_sizes[inc_index])
                            {
                                nc_var_index[inc_index] = 0;
                                inc_index--;
                                nc_var_index[inc_index]++;
                            }
                        }
                    }
                }
            }

            /* Push all subgroups to the list of groups */
            int nc_subgroups;
            nc_err = nc_inq_grps(nc_group, &nc_subgroups, NULL);
            if (nc_err != 0)
                throw exc(namei + ": " + nc_strerror(nc_err));
            if (nc_subgroups > 0)
            {
                std::vector<int> nc_subgroup_ids(nc_subgroups);
                nc_err = nc_inq_grps(nc_group, NULL, &(nc_subgroup_ids[0]));
                if (nc_err != 0)
                    throw exc(namei + ": " + nc_strerror(nc_err));
                for (int g = nc_subgroups - 1; g >= 0; g--)
                    nc_groups.push_front(nc_subgroup_ids[g]);
            }
        }
        array_loop.finish();
        nc_close(nc_file);
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
