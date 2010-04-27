/*
 * lib.h
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

#ifndef LIB_H
#define LIB_H

#include <string>
#include <vector>
#include <cerrno>

#include <gta/gta.hpp>

#include "exc.h"


/* Convert GTA type identifiers to strings and back */
std::string type_to_string(const gta::type t, const uintmax_t size) throw (exc);
void type_from_string(const std::string &s, gta::type *t, uintmax_t *size) throw (exc);

/* Read list of comma-separated types from a string */
void typelist_from_string(const std::string &s, std::vector<gta::type> *types, std::vector<uintmax_t> *sizes) throw (exc);

/* Read list of comma-separated values from a string */
void value_from_string(const std::string &s, const gta::type t, const uintmax_t size, void *value) throw (exc);
void valuelist_from_string(const std::string &s, const std::vector<gta::type> &types,
        const std::vector<uintmax_t> &sizes, void *valuelist) throw (exc);

/* Swap the endianness of a GTA element/component */
void swap_component_endianness(const gta::header &header, uintmax_t i, void *component);
void swap_element_endianness(const gta::header &header, void *element);

/* Convert a linear index to an array of dimension indices */
void linear_index_to_indices(const gta::header &header, uintmax_t e, uintmax_t *indices);
uintmax_t indices_to_linear_index(const gta::header &header, const uintmax_t *indices);

#endif
