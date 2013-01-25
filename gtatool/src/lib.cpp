/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2010, 2011, 2012, 2013
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

#include <limits>
#include <sstream>
#include <cstring>
#include <cstddef>

#include "str.h"
#include "fio.h"
#include "msg.h"
#include "intcheck.h"
#include "endianness.h"
#include "dbg.h"

#include "lib.h"

char *program_name = NULL;
FILE *gtatool_stdin = NULL;
FILE *gtatool_stdout = NULL;


std::string type_to_string(const gta::type t, const uintmax_t size)
{
    std::string s;
    switch (t)
    {
    case gta::blob:
        s = std::string("blob") + str::from(checked_mul(size, static_cast<uintmax_t>(8)));
        break;
    case gta::int8:
        s = "int8";
        break;
    case gta::uint8:
        s = "uint8";
        break;
    case gta::int16:
        s = "int16";
        break;
    case gta::uint16:
        s = "uint16";
        break;
    case gta::int32:
        s = "int32";
        break;
    case gta::uint32:
        s = "uint32";
        break;
    case gta::int64:
        s = "int64";
        break;
    case gta::uint64:
        s = "uint64";
        break;
    case gta::int128:
        s = "int128";
        break;
    case gta::uint128:
        s = "uint128";
        break;
    case gta::float32:
        s = "float32";
        break;
    case gta::float64:
        s = "float64";
        break;
    case gta::float128:
        s = "float128";
        break;
    case gta::cfloat32:
        s = "cfloat32";
        break;
    case gta::cfloat64:
        s = "cfloat64";
        break;
    case gta::cfloat128:
        s = "cfloat128";
        break;
    }
    return s;
}

void type_from_string(const std::string &s, gta::type *t, uintmax_t *size)
{
    if (s.compare(0, 4, "blob") == 0)
    {
        std::istringstream is(s.substr(4));
        uintmax_t x;
        is >> x;
        if (is.fail())
        {
            throw exc(std::string("invalid blob size in ") + s);
        }
        if (x == 0)
        {
            throw exc(std::string("invalid blob size 0 in ") + s);
        }
        if (x % 8 != 0)
        {
            throw exc(std::string("invalid blob size in ") + s + ": must be a multiple of 8");
        }
        *size = x / 8;
        *t = gta::blob;
    }
    else if (s.compare("int8") == 0)
    {
        *t = gta::int8;
    }
    else if (s.compare("uint8") == 0)
    {
        *t = gta::uint8;
    }
    else if (s.compare("int16") == 0)
    {
        *t = gta::int16;
    }
    else if (s.compare("uint16") == 0)
    {
        *t = gta::uint16;
    }
    else if (s.compare("int32") == 0)
    {
        *t = gta::int32;
    }
    else if (s.compare("uint32") == 0)
    {
        *t = gta::uint32;
    }
    else if (s.compare("int64") == 0)
    {
        *t = gta::int64;
    }
    else if (s.compare("uint64") == 0)
    {
        *t = gta::uint64;
    }
    else if (s.compare("int128") == 0)
    {
        *t = gta::int128;
    }
    else if (s.compare("uint128") == 0)
    {
        *t = gta::uint128;
    }
    else if (s.compare("float32") == 0)
    {
        *t = gta::float32;
    }
    else if (s.compare("float64") == 0)
    {
        *t = gta::float64;
    }
    else if (s.compare("float128") == 0)
    {
        *t = gta::float128;
    }
    else if (s.compare("cfloat32") == 0)
    {
        *t = gta::cfloat32;
    }
    else if (s.compare("cfloat64") == 0)
    {
        *t = gta::cfloat64;
    }
    else if (s.compare("cfloat128") == 0)
    {
        *t = gta::cfloat128;
    }
    else
    {
        throw exc("invalid type name " + s);
    }
}

void typelist_from_string(const std::string &s, std::vector<gta::type> *types, std::vector<uintmax_t> *sizes)
{
    types->clear();
    sizes->clear();
    if (s.empty())
    {
        return;
    }
    size_t i = 0;
    do
    {
        size_t comma = s.find(',', i);
        size_t component_len = (comma != std::string::npos) ? comma - i : s.length() - i;
        std::string component = s.substr(i, component_len);
        gta::type t;
        uintmax_t size;
        type_from_string(component, &t, &size);
        types->push_back(t);
        if (t == gta::blob)
        {
            sizes->push_back(size);
        }
        i = (comma != std::string::npos) ? comma + 1 : std::string::npos;
    }
    while (i < std::string::npos);
}

void value_from_string(const std::string &s, const gta::type t, const uintmax_t size, void *value)
{
    std::istringstream is(s);
    switch (t)
    {
    case gta::blob:
        {
            int tv;
            is >> tv;
            if (!is.fail() && (tv < std::numeric_limits<uint8_t>::min() || tv > std::numeric_limits<uint8_t>::max()))
            {
                is.setstate(std::ios::failbit);
            }
            memset(value, tv, checked_cast<size_t>(size));
        }
        break;
    case gta::int8:
        {
            int tv;
            is >> tv;
            if (!is.fail() && (tv < std::numeric_limits<int8_t>::min() || tv > std::numeric_limits<int8_t>::max()))
            {
                is.setstate(std::ios::failbit);
            }
            int8_t v = tv;
            memcpy(value, &v, sizeof(int8_t));
        }
        break;
    case gta::uint8:
        {
            int tv;
            is >> tv;
            if (!is.fail() && (tv < std::numeric_limits<uint8_t>::min() || tv > std::numeric_limits<uint8_t>::max()))
            {
                is.setstate(std::ios::failbit);
            }
            uint8_t v = tv;
            memcpy(value, &v, sizeof(uint8_t));
        }
        break;
    case gta::int16:
        {
            int16_t v;
            is >> v;
            memcpy(value, &v, sizeof(int16_t));
        }
        break;
    case gta::uint16:
        {
            uint16_t v;
            is >> v;
            memcpy(value, &v, sizeof(uint16_t));
        }
        break;
    case gta::int32:
        {
            int32_t v;
            is >> v;
            memcpy(value, &v, sizeof(int32_t));
        }
        break;
    case gta::uint32:
        {
            uint32_t v;
            is >> v;
            memcpy(value, &v, sizeof(uint32_t));
        }
        break;
    case gta::int64:
        {
            int64_t v;
            is >> v;
            memcpy(value, &v, sizeof(int64_t));
        }
        break;
    case gta::uint64:
        {
            uint64_t v;
            is >> v;
            memcpy(value, &v, sizeof(uint64_t));
        }
        break;
    case gta::int128:
    case gta::uint128:
        throw exc("128 bit integer types are currently not supported");
        break;
    case gta::float32:
        {
            float v;
            is >> v;
            memcpy(value, &v, sizeof(float));
        }
        break;
    case gta::float64:
        {
            double v;
            is >> v;
            memcpy(value, &v, sizeof(double));
        }
        break;
    case gta::float128:
    case gta::cfloat128:
        throw exc("the quad-precision floating point type is currently not supported");
        break;
    case gta::cfloat32:
        {
            char *float_value = static_cast<char *>(value);
            float v0;
            is >> v0;
            memcpy(float_value + 0, &v0, sizeof(float));
            char comma = '\0';
            is >> comma;
            if (comma != ',')
            {
                throw exc("two comma separated values expected for complex types");
            }
            float v1;
            is >> v1;
            memcpy(float_value + sizeof(float), &v1, sizeof(float));
        }
        break;
    case gta::cfloat64:
        {
            char *double_value = static_cast<char *>(value);
            double v0;
            is >> v0;
            memcpy(double_value + 0, &v0, sizeof(double));
            char comma = '\0';
            is >> comma;
            if (comma != ',')
            {
                throw exc("two comma separated values expected for complex types");
            }
            double v1;
            is >> v1;
            memcpy(double_value + sizeof(double), &v1, sizeof(double));
        }
        break;
    }
    if (is.fail() || !is.eof())
    {
        throw exc(std::string("cannot read ") + type_to_string(t, 0) + " from " + str::sanitize(s), EINVAL);
    }
}

void valuelist_from_string(const std::string &s, const std::vector<gta::type> &types,
        const std::vector<uintmax_t> &sizes, void *valuelist)
{
    if ((types.size() == 0 && !s.empty()) || (types.size() > 0 && s.empty()))
    {
        throw exc("invalid number of values");
    }
    size_t i = 0;
    size_t blob_index = 0;
    char *value = static_cast<char *>(valuelist);
    for (size_t j = 0; j < types.size(); j++)
    {
        size_t comma = s.find(',', i);
        if (types[j] == gta::cfloat32 || types[j] == gta::cfloat64 || types[j] == gta::cfloat128)
        {
            if (comma != std::string::npos)
            {
                comma = s.find(',', comma + 1);
            }
        }
        if ((j == types.size() - 1 && comma != std::string::npos)
                || (j < types.size() - 1 && comma == std::string::npos))
        {
            throw exc("invalid number of values");
        }
        size_t component_len = (comma != std::string::npos) ? comma - i : s.length() - i;
        std::string component = s.substr(i, component_len);
        value_from_string(component, types[j], 
                types[j] == gta::blob ? sizes[blob_index] : 0,
                value);
        i = (comma != std::string::npos) ? comma + 1 : std::string::npos;
        switch (types[j])
        {
        case gta::blob:
            value += checked_cast<size_t>(sizes[blob_index]);
            blob_index++;
            break;
        case gta::int8:
        case gta::uint8:
            value += 1;
            break;
        case gta::int16:
        case gta::uint16:
            value += 2;
            break;
        case gta::int32:
        case gta::uint32:
        case gta::float32:
            value += 4;
            break;
        case gta::int64:
        case gta::uint64:
        case gta::float64:
        case gta::cfloat32:
            value += 8;
            break;
        case gta::int128:
        case gta::uint128:
        case gta::float128:
        case gta::cfloat64:
            value += 16;
            break;
        case gta::cfloat128:
            value += 32;
            break;
        }
    }
}


void swap_component_endianness(const gta::header &header, uintmax_t i, void *component)
{
    switch (header.component_type(i))
    {
    case gta::blob:
    case gta::int8:
    case gta::uint8:
        break;
    case gta::int16:
    case gta::uint16:
        endianness::swap16(component);
        break;
    case gta::int32:
    case gta::uint32:
    case gta::float32:
        endianness::swap32(component);
        break;
    case gta::int64:
    case gta::uint64:
    case gta::float64:
        endianness::swap64(component);
        break;
    case gta::int128:
    case gta::uint128:
    case gta::float128:
        endianness::swap128(component);
        break;
    case gta::cfloat32:
        {
            uint32_t *u32 = static_cast<uint32_t *>(component);
            endianness::swap32(u32 + 0);
            endianness::swap32(u32 + 1);
        }
        break;
    case gta::cfloat64:
        {
            uint64_t *u64 = static_cast<uint64_t *>(component);
            endianness::swap64(u64 + 0);
            endianness::swap64(u64 + 1);
        }
        break;
    case gta::cfloat128:
        {
            uint64_t *u64 = static_cast<uint64_t *>(component);
            endianness::swap128(u64 + 0);
            endianness::swap128(u64 + 2);
        }
        break;
    }
}

void swap_element_endianness(const gta::header &header, void *element)
{
    char *ptr = static_cast<char *>(element);
    for (uintmax_t i = 0; i < header.components(); i++)
    {
        swap_component_endianness(header, i, ptr);
        ptr += header.component_size(i);
    }
}

std::string from_utf8(const std::string &s)
{
    const std::string localcharset = str::localcharset();
    std::string r;
    try
    {
        r = str::convert(s, "UTF-8", localcharset);
    }
    catch (...)
    {
        r = std::string("(not representable in charset ") + localcharset + std::string(")");
    }
    return r;
}

std::string to_utf8(const std::string &s)
{
    const std::string localcharset = str::localcharset();
    std::string r;
    try
    {
        r = str::convert(s, localcharset, "UTF-8");
    }
    catch (std::exception &e)
    {
        /* This should never happen: everything that can be entered by the user should
         * be representable in UTF-8. Crash the program and let's see if this ever gets
         * reported. */
        msg::err("CANNOT CONVERT TO UTF-8: %s", e.what());
        dbg::crash();
    }
    return r;
}

const size_t element_loop_t::_max_iobuf_size = 1024 * 1024;

element_loop_t::element_loop_t() throw ()
    : _header_in(), _name_in(), _file_in(NULL), _state_in(),
    _header_out(), _name_out(), _file_out(NULL), _state_out(), _buf()
{
}

element_loop_t::~element_loop_t()
{
}

void element_loop_t::start(
        const gta::header &header_in, const std::string &name_in, FILE *file_in,
        const gta::header &header_out, const std::string &name_out, FILE *file_out)
{
    _header_in = header_in;
    _name_in = name_in;
    _file_in = file_in;
    _state_in = gta::io_state();
    _header_out = header_out;
    _name_out = name_out;
    _file_out = file_out;
    _state_out = gta::io_state();
    _buf.resize(0);
}

const void *element_loop_t::read(size_t n)
{
    if (_buf.size() < checked_cast<size_t>(n * _header_in.element_size()))
    {
        _buf.resize(n * _header_in.element_size());
    }
    _header_in.read_elements(_state_in, _file_in, n, _buf.ptr());
    return _buf.ptr();
}

void element_loop_t::write(const void *element, size_t n)
{
    _header_out.write_elements(_state_out, _file_out, n, element);
}

const std::string array_loop_t::_stdin_name = "standard input";
const std::string array_loop_t::_stdout_name = "standard output";

array_loop_t::array_loop_t() throw ()
    : _filenames_in(), _filename_out(),
    _file_in(NULL), _file_out(NULL),
    _filename_index(0), _file_index_in(0),
    _index_in(0), _index_out(0),
    _array_name_in(), _array_name_out()
{
}

array_loop_t::~array_loop_t()
{
    if (_file_in && _file_in != gtatool_stdin)
    {
        try
        {
            fio::close(_file_in, filename_in());
        }
        catch (...)
        {
        }
    }
    if (_file_out && _file_out != gtatool_stdout)
    {
        try
        {
            fio::close(_file_out, filename_out());
        }
        catch (...)
        {
        }
    }
}

void array_loop_t::start(const std::vector<std::string> &filenames_in,
        const std::string &filename_out)
{
    _filenames_in = filenames_in;
    _filename_out = filename_out;
    _file_in = NULL;
    _file_out = NULL;
    _filename_index = 0;
    _file_index_in = 0;
    _index_in = 0;
    _index_out = 0;
    _array_name_in = "";
    _array_name_out = "";
    if (_filenames_in.size() == 0)
    {
        _file_in = gtatool_stdin;
    }
    else
    {
        _file_in = fio::open(_filenames_in[0], "r");
    }
    if (_filename_out.length() == 0)
    {
        _file_out = gtatool_stdout;
    }
    else
    {
        _file_out = fio::open(_filename_out, "w");
    }
}

void array_loop_t::finish()
{
    if (_file_out && _file_out != gtatool_stdout)
    {
        FILE *f = _file_out;
        const std::string &name = filename_out();
        _file_out = NULL;
        fio::close(f, name);
    }
    if (_file_in && _file_in != gtatool_stdin)
    {
        FILE *f = _file_in;
        const std::string &name = filename_in();
        _file_in = NULL;
        fio::close(f, name);
    }
}

const std::string &array_loop_t::filename_in() throw ()
{
    return (_file_in == gtatool_stdin ? _stdin_name : _filenames_in[_filename_index]);
}

const std::string &array_loop_t::filename_out() throw ()
{
    return (_file_out == gtatool_stdout ? _stdout_name : _filename_out);
}

bool array_loop_t::read(gta::header &header_in, std::string &name_in)
{
    while (!fio::has_more(_file_in, filename_in()))
    {
        if (_filenames_in.size() == 0)
        {
            return false;
        }
        else
        {
            if (_filename_index + 1 == _filenames_in.size())
            {
                return false;
            }
            FILE *f = _file_in;
            _file_in = NULL;
            fio::close(f, filename_in());
            _filename_index++;
            _file_in = fio::open(_filenames_in.at(_filename_index), "r");
            _file_index_in = 0;
        }
    }
    _array_name_in = filename_in() + " array " + str::from(_file_index_in);
    name_in = _array_name_in;
    try
    {
        header_in.read_from(_file_in);
    }
    catch (std::exception &e)
    {
        throw exc(_array_name_in + ": " + e.what());
    }
    _file_index_in++;
    _index_in++;
    return true;
}

void array_loop_t::write(const gta::header &header_out, std::string &name_out)
{
    if (fio::isatty(_file_out))
    {
        throw exc("refusing to write to a tty");
    }
    _array_name_out = filename_out() + " array " + str::from(_index_out);
    name_out = _array_name_out;
    try
    {
        header_out.write_to(_file_out);
    }
    catch (std::exception &e)
    {
        throw exc(_array_name_out + ": " + e.what());
    }
    _index_out++;
}

void array_loop_t::skip_data(const gta::header &header_in)
{
    try
    {
        header_in.skip_data(_file_in);
    }
    catch (std::exception &e)
    {
        throw exc(_array_name_in + ": " + e.what());
    }
}

void array_loop_t::copy_data(const gta::header &header_in, const gta::header &header_out)
{
    try
    {
        header_in.copy_data(_file_in, header_out, _file_out);
    }
    catch (std::exception &e)
    {
        throw exc(_array_name_in + ": " + e.what());
    }
}

void array_loop_t::copy_data(const gta::header &header_in, const array_loop_t &array_loop_out, gta::header &header_out)
{
    try
    {
        header_in.copy_data(_file_in, header_out, array_loop_out._file_out);
    }
    catch (std::exception &e)
    {
        throw exc(_array_name_in + ": " + e.what());
    }
}

void array_loop_t::read_data(const gta::header &header_in, void *data)
{
    try
    {
        header_in.read_data(_file_in, data);
    }
    catch (std::exception &e)
    {
        throw exc(_array_name_in + ": " + e.what());
    }
}

void array_loop_t::write_data(const gta::header &header_out, const void *data)
{
    try
    {
        header_out.write_data(_file_out, data);
    }
    catch (std::exception &e)
    {
        throw exc(_array_name_in + ": " + e.what());
    }
}

void array_loop_t::start_element_loop(element_loop_t &element_loop,
        const gta::header &header_in, const gta::header &header_out)
{
    element_loop.start(header_in, _array_name_in, _file_in,
            header_out, _array_name_out, _file_out);
}

void buffer_data(const gta::header &header, FILE *f, gta::header &buf_header, FILE **buf_f)
{
    *buf_f = fio::tempfile();
    buf_header = header;
    buf_header.set_compression(gta::none);
    header.copy_data(f, buf_header, *buf_f);
}
