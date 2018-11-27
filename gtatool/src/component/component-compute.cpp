/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010, 2011, 2013, 2014, 2018
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
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <unistd.h>

#include <gta/gta.hpp>

#include <muParser.h>

#include "base/dbg.h"
#include "base/msg.h"
#include "base/blb.h"
#include "base/opt.h"
#include "base/fio.h"
#include "base/str.h"
#include "base/chk.h"

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
            "The expressions are evaluated using the muParser library, with additions taken from mucalc. See "
            "<https://gitlab.marlam.de/marlam/mucalc> for an overview "
            "of functions and operators that can be used.\n"
            "Example: component-compute -e 'c3 = 0.2126 * c0 + 0.7152 * c1 + 0.0722 * c2' rgba.gta > rgb+lum.gta");
}

/* muparser custom constants */

static const double e = 2.7182818284590452353602874713526625;
static const double pi = 3.1415926535897932384626433832795029;

/* muparser custom operators */

static double mod(double x, double y)
{
    return x - y * floor(x / y);
}

/* muparser custom functions */

static double deg(double x)
{
    return x * 180.0 / pi;
}

static double rad(double x)
{
    return x * pi / 180.0;
}

static double int_(double x)
{
    return (int)x;
}

static double fract(double x)
{
    return x - floor(x);
}

static double med(const double* x, int n)
{
    std::vector<double> values(x, x + n);
    std::sort(values.begin(), values.end());
    if (n % 2 == 1) {
        return values[n / 2];
    } else {
        return (values[n / 2 - 1] + values[n / 2]) / 2.0;
    }
}

static double clamp(double x, double minval, double maxval)
{
    return std::min(maxval, std::max(minval, x));
}

static double step(double x, double edge)
{
    return (x < edge ? 0.0 : 1.0);
}

static double smoothstep(double x, double edge0, double edge1)
{
    double t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - t * 2.0);
}

static double mix(double x, double y, double t)
{
    return x * (1.0 - t) + y * t;
}

static double unary_plus(double x)
{
    return x;
}

static double my_srand48(double x)
{
    srand48(x);
    return x;
}

static double my_random()
{
    static bool initialized = false;
    static unsigned short xsubi[3];
    if (!initialized) {
        FILE* f = fopen("/dev/urandom", "r");
        if (f) {
            setbuf(f, NULL);
            fread(xsubi, sizeof(unsigned short), 3, f);
            fclose(f);
        } else {
            xsubi[0] = 0x330E;
            xsubi[1] = getpid() & 0xffff;
            xsubi[2] = time(NULL) & 0xffff;
        }
        initialized = true;
    }
    return erand48(xsubi);
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
                if (hdri.component_type(i) == gta::blob)
                {
                    throw exc(namei + ": cannot compute variables of type "
                            + type_to_string(hdri.component_type(i), hdri.component_size(i)));
                }
                if (false
#ifndef HAVE_INT128_T
                        || hdri.component_type(i) == gta::int128
#endif
#ifndef HAVE_UINT128_T
                        || hdri.component_type(i) == gta::uint128
#endif
#ifndef HAVE_FLOAT128_T
                        || hdri.component_type(i) == gta::float128
                        || hdri.component_type(i) == gta::cfloat128
#endif
                   )
                {
                    throw exc(namei + ": cannot compute variables of type "
                            + type_to_string(hdri.component_type(i), hdri.component_size(i))
                            + " on this platform");
                }
                if (hdri.component_type(i) == gta::cfloat32
                        || hdri.component_type(i) == gta::cfloat64
                        || hdri.component_type(i) == gta::cfloat128)
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
                parsers[p].ClearConst();
                parsers[p].DefineConst("e", e);
                parsers[p].DefineConst("pi", pi);
                parsers[p].DefineOprt("%", mod, mu::prMUL_DIV, mu::oaLEFT, true);
                parsers[p].DefineFun("deg", deg);
                parsers[p].DefineFun("rad", rad);
                parsers[p].DefineFun("atan2", atan2);
                parsers[p].DefineFun("fract", fract);
                parsers[p].DefineFun("pow", pow);
                parsers[p].DefineFun("exp2", exp2);
                parsers[p].DefineFun("cbrt", cbrt);
                parsers[p].DefineFun("int", int_);
                parsers[p].DefineFun("ceil", ceil);
                parsers[p].DefineFun("floor", floor);
                parsers[p].DefineFun("round", round);
                parsers[p].DefineFun("trunc", trunc);
                parsers[p].DefineFun("med", med);
                parsers[p].DefineFun("clamp", clamp);
                parsers[p].DefineFun("step", step);
                parsers[p].DefineFun("smoothstep", smoothstep);
                parsers[p].DefineFun("mix", mix);
                parsers[p].DefineFun("random", my_random, false);
                parsers[p].DefineFun("srand48", my_srand48, false);
                parsers[p].DefineFun("drand48", drand48, false);
                parsers[p].DefineInfixOprt("+", unary_plus);
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
#ifdef HAVE_INT128_T
                        case gta::int128:
                            {
                                int128_t v;
                                std::memcpy(&v, hdri.component(element.ptr(), i), sizeof(int128_t));
                                comp_vars[comp_var_index++] = v;
                            }
                            break;
#endif
#ifdef HAVE_UINT128_T
                        case gta::uint128:
                            {
                                uint128_t v;
                                std::memcpy(&v, hdri.component(element.ptr(), i), sizeof(uint128_t));
                                comp_vars[comp_var_index++] = v;
                            }
                            break;
#endif
                        case gta::float32:
                            {
                                float v;
                                std::memcpy(&v, hdri.component(element.ptr(), i), sizeof(float));
                                comp_vars[comp_var_index++] = v;
                            }
                            break;
                        case gta::float64:
                            {
                                double v;
                                std::memcpy(&v, hdri.component(element.ptr(), i), sizeof(double));
                                comp_vars[comp_var_index++] = v;
                            }
                            break;
#ifdef HAVE_FLOAT128_T
                        case gta::float128:
                            {
                                float128_t v;
                                std::memcpy(&v, hdri.component(element.ptr(), i), sizeof(float128_t));
                                comp_vars[comp_var_index++] = v;
                            }
                            break;
#endif
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
#ifdef HAVE_FLOAT128_T
                        case gta::cfloat128:
                            {
                                float128_t v[2];
                                std::memcpy(v, hdri.component(element.ptr(), i), 2 * sizeof(float128_t));
                                comp_vars[comp_var_index++] = v[0];
                                comp_vars[comp_var_index++] = v[1];
                            }
                            break;
#endif
                        default:
                            // cannot happen
                            assert(false);
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
#ifdef HAVE_INT128_T
                        case gta::int128:
                            {
                                int128_t v = comp_vars[comp_var_index++];
                                std::memcpy(hdri.component(element.ptr(), i), &v, sizeof(int128_t));
                            }
                            break;
#endif
#ifdef HAVE_UINT128_T
                        case gta::uint128:
                            {
                                uint128_t v = comp_vars[comp_var_index++];
                                std::memcpy(hdri.component(element.ptr(), i), &v, sizeof(uint128_t));
                            }
                            break;
#endif
                        case gta::float32:
                            {
                                float v = comp_vars[comp_var_index++];
                                std::memcpy(hdri.component(element.ptr(), i), &v, sizeof(float));
                            }
                            break;
                        case gta::float64:
                            {
                                double v = comp_vars[comp_var_index++];
                                std::memcpy(hdri.component(element.ptr(), i), &v, sizeof(double));
                            }
                            break;
#ifdef HAVE_FLOAT128_T
                        case gta::float128:
                            {
                                float128_t v = comp_vars[comp_var_index++];
                                std::memcpy(hdri.component(element.ptr(), i), &v, sizeof(float128_t));
                            }
                            break;
#endif
                        case gta::cfloat32:
                            {
                                float v[2] = { static_cast<float>(comp_vars[comp_var_index]), static_cast<float>(comp_vars[comp_var_index + 1]) };
                                comp_var_index += 2;
                                std::memcpy(hdri.component(element.ptr(), i), v, 2 * sizeof(float));
                            }
                            break;
                        case gta::cfloat64:
                            {
                                double v[2] = { comp_vars[comp_var_index], comp_vars[comp_var_index + 1] };
                                comp_var_index += 2;
                                std::memcpy(hdri.component(element.ptr(), i), v, 2 * sizeof(double));
                            }
                            break;
#ifdef HAVE_FLOAT128_T
                        case gta::cfloat128:
                            {
                                float128_t v[2] = { comp_vars[comp_var_index], comp_vars[comp_var_index + 1] };
                                comp_var_index += 2;
                                std::memcpy(hdri.component(element.ptr(), i), v, 2 * sizeof(float128_t));
                            }
                            break;
#endif
                        default:
                            // cannot happen
                            assert(false);
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
