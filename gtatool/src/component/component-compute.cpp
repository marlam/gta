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

#include <muParser.h>

#include "msg.h"
#include "blob.h"
#include "opt.h"
#include "fio.h"
#include "str.h"
#include "intcheck.h"

#include "lib.h"


extern "C" void gtatool_component_compute_help(void)
{
    msg::req_txt(
            "component-compute -e|--expression=<exp0> [-e|--expression=<exp1> [...]] [<files>...]\n"
            "\n"
            "Compute array element components. For each array element in an input GTA with n array element components, "
            "the components c0..c(n-1) can be recomputed using the given expression(s). "
            "All computations are done in double precision floating point, regardless of the original component type. "
            "For complex components (cfloat), there are two component variables for the real and imaginary part, e.g. "
            "c4re and c4im for component 4. In addition to the modifiable variables c0..c(n-1), the following "
            "non-modifiable variables are defined: c (the number of components of an array element), "
            "d (the number of dimensions of the array), d0..d(d-1) (the array size in each dimension), "
            "i0..i(d-1) (the index of the current array element).\n"
            "The expressions are evaluated using the muParser library; see <http://muparser.sourceforge.net/> for an "
            "overview of functions and operators that can be used.\n"
            "Example: component-compute -e 'c3 = 0.2126 * c0 + 0.7152 * c1 + 0.0722 * c2' rgba.gta > rgb+lum.gta");
}

extern "C" int gtatool_component_compute(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::string expressions("expression", 'e', opt::required);
    options.push_back(&expressions);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, -1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_component_compute_help();
        return 0;
    }

    try
    {
        array_loop_t array_loop;
        gta::header hdri, hdro;
        std::string namei, nameo;
        array_loop.start(arguments, "");
        while (array_loop.read(hdri, namei))
        {
            // Set up variables
            std::vector<double> comp_vars;
            for (uintmax_t i = 0; i < hdri.components(); i++)
            {
                if (hdri.component_type(i) == gta::blob
                        || hdri.component_type(i) == gta::int128
                        || hdri.component_type(i) == gta::uint128
                        || hdri.component_type(i) == gta::float128
                        || hdri.component_type(i) == gta::cfloat128)
                {
                    throw exc(namei + ": cannot compute variables of type "
                            + type_to_string(hdri.component_type(i), hdri.component_size(i)));
                }
                if (hdri.component_type(i) == gta::cfloat32 || hdri.component_type(i) == gta::cfloat64)
                {
                    comp_vars.push_back(0.0);
                    comp_vars.push_back(0.0);
                }
                else
                {
                    comp_vars.push_back(0.0);
                }
            }
            double components_var;
            double dimensions_var;
            std::vector<double> dim_vars(checked_cast<size_t>(hdri.dimensions()));
            std::vector<uintmax_t> index_vars_orig(hdri.dimensions());
            std::vector<double> index_vars(hdri.dimensions());
            std::vector<mu::Parser> parsers;
            parsers.resize(expressions.values().size());
            for (size_t p = 0; p < expressions.values().size(); p++)
            {
                size_t comp_vars_index = 0;
                for (uintmax_t i = 0; i < hdri.components(); i++)
                {
                    if (hdri.component_type(i) == gta::cfloat32 || hdri.component_type(i) == gta::cfloat64)
                    {
                        parsers[p].DefineVar(std::string("c") + str::from(i) + "re", &(comp_vars[comp_vars_index++]));
                        parsers[p].DefineVar(std::string("c") + str::from(i) + "im", &(comp_vars[comp_vars_index++]));
                    }
                    else
                    {
                        parsers[p].DefineVar(std::string("c") + str::from(i), &(comp_vars[comp_vars_index++]));
                    }
                }
                parsers[p].DefineVar("c", &components_var);
                parsers[p].DefineVar("d", &dimensions_var);
                for (uintmax_t i = 0; i < hdri.dimensions(); i++)
                {
                    parsers[p].DefineVar(std::string("d") + str::from(i), &(dim_vars[i]));
                    parsers[p].DefineVar(std::string("i") + str::from(i), &(index_vars[i]));
                }
                parsers[p].SetExpr(expressions.values()[p]);
            }

            hdro = hdri;
            hdro.set_compression(gta::none);
            array_loop.write(hdro, nameo);
            if (hdro.data_size() > 0)
            {
                element_loop_t element_loop;
                array_loop.start_element_loop(element_loop, hdri, hdro);
                blob element(checked_cast<size_t>(hdri.element_size()));
                for (uintmax_t e = 0; e < hdro.elements(); e++)
                {
                    std::memcpy(element.ptr(), element_loop.read(), hdri.element_size());
                    // set the variables
                    components_var = hdri.components();
                    dimensions_var = hdri.dimensions();
                    for (uintmax_t i = 0; i < hdri.dimensions(); i++)
                    {
                        dim_vars[i] = hdri.dimension_size(i);
                    }
                    hdri.linear_index_to_indices(e, &(index_vars_orig[0]));
                    for (uintmax_t i = 0; i < hdri.dimensions(); i++)
                    {
                        index_vars[i] = index_vars_orig[i];
                    }
                    size_t comp_var_index = 0;
                    for (uintmax_t i = 0; i < hdri.components(); i++)
                    {
                        switch (hdri.component_type(i))
                        {
                        case gta::int8:
                            {
                                int8_t v;
                                std::memcpy(&v, hdri.component(element.ptr(), i), sizeof(int8_t));
                                comp_vars[comp_var_index++] = v;
                            }
                            break;
                        case gta::uint8:
                            {
                                uint8_t v;
                                std::memcpy(&v, hdri.component(element.ptr(), i), sizeof(uint8_t));
                                comp_vars[comp_var_index++] = v;
                            }
                            break;
                        case gta::int16:
                            {
                                int16_t v;
                                std::memcpy(&v, hdri.component(element.ptr(), i), sizeof(int16_t));
                                comp_vars[comp_var_index++] = v;
                            }
                            break;
                        case gta::uint16:
                            {
                                uint16_t v;
                                std::memcpy(&v, hdri.component(element.ptr(), i), sizeof(uint16_t));
                                comp_vars[comp_var_index++] = v;
                            }
                            break;
                        case gta::int32:
                            {
                                int32_t v;
                                std::memcpy(&v, hdri.component(element.ptr(), i), sizeof(int32_t));
                                comp_vars[comp_var_index++] = v;
                            }
                            break;
                        case gta::uint32:
                            {
                                uint32_t v;
                                std::memcpy(&v, hdri.component(element.ptr(), i), sizeof(uint32_t));
                                comp_vars[comp_var_index++] = v;
                            }
                            break;
                        case gta::int64:
                            {
                                int64_t v;
                                std::memcpy(&v, hdri.component(element.ptr(), i), sizeof(int64_t));
                                comp_vars[comp_var_index++] = v;
                            }
                            break;
                        case gta::uint64:
                            {
                                uint64_t v;
                                std::memcpy(&v, hdri.component(element.ptr(), i), sizeof(uint64_t));
                                comp_vars[comp_var_index++] = v;
                            }
                            break;
                        case gta::float32:
                            {
                                float v;
                                std::memcpy(&v, hdri.component(element.ptr(), i), sizeof(float));
                                comp_vars[comp_var_index++] = v;
                            }
                            break;
                        case gta::float64:
                            {
                                std::memcpy(&(comp_vars[comp_var_index++]), hdri.component(element.ptr(), i), sizeof(double));
                            }
                            break;
                        case gta::cfloat32:
                            {
                                float v[2];
                                std::memcpy(v, hdri.component(element.ptr(), i), 2 * sizeof(float));
                                comp_vars[comp_var_index++] = v[0];
                                comp_vars[comp_var_index++] = v[1];
                            }
                            break;
                        case gta::cfloat64:
                            {
                                double v[2];
                                std::memcpy(v, hdri.component(element.ptr(), i), 2 * sizeof(double));
                                comp_vars[comp_var_index++] = v[0];
                                comp_vars[comp_var_index++] = v[1];
                            }
                            break;
                        default:
                            // cannot happen
                            break;
                        }
                    }
                    // evaluate the expressions
                    for (size_t p = 0; p < parsers.size(); p++)
                    {
                        parsers[p].Eval();
                    }
                    // read back the component variables
                    comp_var_index = 0;
                    for (uintmax_t i = 0; i < hdro.components(); i++)
                    {
                        switch (hdro.component_type(i))
                        {
                        case gta::int8:
                            {
                                int8_t v = comp_vars[comp_var_index++];
                                std::memcpy(hdri.component(element.ptr(), i), &v, sizeof(int8_t));
                            }
                            break;
                        case gta::uint8:
                            {
                                uint8_t v = comp_vars[comp_var_index++];
                                std::memcpy(hdri.component(element.ptr(), i), &v, sizeof(uint8_t));
                            }
                            break;
                        case gta::int16:
                            {
                                int16_t v = comp_vars[comp_var_index++];
                                std::memcpy(hdri.component(element.ptr(), i), &v, sizeof(int16_t));
                            }
                            break;
                        case gta::uint16:
                            {
                                uint16_t v = comp_vars[comp_var_index++];
                                std::memcpy(hdri.component(element.ptr(), i), &v, sizeof(uint16_t));
                            }
                            break;
                        case gta::int32:
                            {
                                int32_t v = comp_vars[comp_var_index++];
                                std::memcpy(hdri.component(element.ptr(), i), &v, sizeof(int32_t));
                            }
                            break;
                        case gta::uint32:
                            {
                                uint32_t v = comp_vars[comp_var_index++];
                                std::memcpy(hdri.component(element.ptr(), i), &v, sizeof(uint32_t));
                            }
                            break;
                        case gta::int64:
                            {
                                int64_t v = comp_vars[comp_var_index++];
                                std::memcpy(hdri.component(element.ptr(), i), &v, sizeof(int64_t));
                            }
                            break;
                        case gta::uint64:
                            {
                                uint64_t v = comp_vars[comp_var_index++];
                                std::memcpy(hdri.component(element.ptr(), i), &v, sizeof(uint64_t));
                            }
                            break;
                        case gta::float32:
                            {
                                float v = comp_vars[comp_var_index++];
                                std::memcpy(hdri.component(element.ptr(), i), &v, sizeof(float));
                            }
                            break;
                        case gta::float64:
                            {
                                std::memcpy(hdri.component(element.ptr(), i), &(comp_vars[comp_var_index++]), sizeof(double));
                            }
                            break;
                        case gta::cfloat32:
                            {
                                float v[2] = { static_cast<float>(comp_vars[comp_var_index]), static_cast<float>(comp_vars[comp_var_index + 1]) };
                                comp_var_index += 2;
                                std::memcpy(hdri.component(element.ptr(), i), v, 2 * sizeof(float));
                            }
                            break;
                        case gta::cfloat64:
                            {
                                std::memcpy(hdri.component(element.ptr(), i), &(comp_vars[comp_var_index]), 2 * sizeof(double));
                                comp_var_index += 2;
                            }
                            break;
                        default:
                            // cannot happen
                            break;
                        }
                    }
                    element_loop.write(element.ptr());
                }
            }
        }
        array_loop.finish();
    }
    catch (mu::Parser::exception_type &e)
    {
        msg::err_txt(e.GetMsg());
        return 1;
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        return 1;
    }

    return 0;
}
