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
#include <cstdio>

#include <gta/gta.hpp>

#include "exc.h"
#include "blob.h"

// The name of the binary of this program.
extern char *program_name;

/* We need to redirect stdin/stdout when calling commands from the GUI.
 * However, assigning override values to stdin/stdout is not portable because
 * the standard streams need not be lvalues.
 * To keep things simple, we only use gtatool_stdin and gtatool_stdout in the
 * command implementations, and set these variables from main.cpp (command line
 * interface) and gui.cpp (GUI interface). */
extern FILE *gtatool_stdin;
extern FILE *gtatool_stdout;

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

/* Convert strings between the local character set and UTF-8, in a fail-safe way */
std::string from_utf8(const std::string &s);
std::string to_utf8(const std::string &s);

/* Loop over all input and output array elements.
 * This loop provides input/output buffering for filtering commands that
 * work on array element level. */
class element_loop_t
{
private:
    static const size_t _max_iobuf_size;

    gta::header _header_in;
    std::string _name_in;
    FILE *_file_in;
    gta::header _header_out;
    std::string _name_out;
    FILE *_file_out;

    gta::io_state _state_in;
    uintmax_t _element_index_in;
    blob _buf_in;
    uintmax_t _buf_elements_in;
    uintmax_t _buf_index_in;
    gta::io_state _state_out;
    uintmax_t _element_index_out;
    blob _buf_out;
    uintmax_t _buf_elements_out;
    uintmax_t _buf_index_out;

public:
    element_loop_t() throw ();
    element_loop_t(const gta::header &header_in, const std::string &name_in, FILE *file_in,
            const gta::header &header_out, const std::string &name_out, FILE *file_out) throw (exc);
    ~element_loop_t();

    void *read() throw (exc);
    void write(const void *element) throw (exc);

    void finish() throw (exc);
};

/* Loop over all input and output arrays.
 * The input arrays usually come from multiple files, or possibly an input stream
 * if the list of files is empty. This input stream is usually stdin.
 * The output arrays usually go into a single stream (often stdout).
 * This loop is general enough to be usable by all filtering commands. */
class array_loop_t
{
private:
    static const std::string _stdin_name;
    static const std::string _stdout_name;

    const std::vector<std::string> _filenames_in;
    const std::string _filename_out;

    FILE *_file_in;
    FILE *_file_out;
    size_t _filename_index;
    uintmax_t _file_index_in;
    uintmax_t _index_in;
    uintmax_t _index_out;
    std::string _array_name_in;
    std::string _array_name_out;

    const std::string &filename_in() throw ();
    const std::string &filename_out() throw ();

public:
    array_loop_t(const std::vector<std::string> &filenames_in,
            const std::string &filename_out) throw (exc);
    ~array_loop_t();

    bool read(gta::header &header_in, std::string &name_in) throw (exc);

    void write(const gta::header &header_out, std::string &name_out) throw (exc);

    void skip_data(const gta::header &header_in) throw (exc);
    void copy_data(const gta::header &header_in, const gta::header &header_out) throw (exc);
    element_loop_t element_loop(const gta::header &header_in, const gta::header &header_out) throw (exc);

    void finish() throw (exc);
};

#endif
