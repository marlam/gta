/*
 * exc.h
 *
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2006, 2007, 2008, 2009, 2010
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

#include <cstring>
#include <cerrno>

#include "msg.h"
#include "exc.h"


void exc::create(const char *when, int sys_errno, const char *what) throw ()
{
    _str[0] = '\0';
    _sys_errno = sys_errno;
    size_t i = 0;
    if (when[0] != '\0')
    {
        strncpy(_str, when, _strbufsize);
        _str[_strbufsize - 1] = '\0';
        i = strlen(_str);
    }
    if (what[0] != '\0')
    {
        if (when[0] != '\0')
        {
            strncpy(_str + i, ": ", _strbufsize - i);
            _str[_strbufsize - 1] = '\0';
            i = strlen(_str);
        }
        strncpy(_str + i, what, _strbufsize - i);
        _str[_strbufsize - 1] = '\0';
    }
    else if (sys_errno != 0)
    {
        if (when[0] != '\0')
        {
            strncpy(_str + i, ": ", _strbufsize - i);
            _str[_strbufsize - 1] = '\0';
            i = strlen(_str);
        }
        strncpy(_str + i, ::strerror(_sys_errno), _strbufsize - i);
        _str[_strbufsize - 1] = '\0';
    }
}

exc::exc() throw ()
    : std::exception()
{
    _str[0] = '\0';
    _sys_errno = 0;
}

exc::exc(const std::string &when, int sys_errno, const std::string &what) throw ()
    : std::exception()
{
    create(when.c_str(), sys_errno, what.c_str());
    if (!empty())
    {
        msg::dbg("Exception: %s", _str);
    }
}

exc::exc(const std::string &when, const std::string &what) throw ()
    : std::exception()
{
    create(when.c_str(), 0, what.c_str());
    if (!empty())
    {
        msg::dbg("Exception: %s", _str);
    }
}

exc::exc(int sys_errno) throw ()
    : std::exception()
{
    create("", sys_errno, "");
    if (!empty())
    {
        msg::dbg("Exception: %s", _str);
    }
}

exc::exc(const exc &e) throw ()
    : std::exception()
{
    strcpy(_str, e._str);
}

exc::exc(const std::exception &e) throw ()
    : std::exception()
{
    // TODO: Avoid the crappy what() strings; ideally translate them to errnos.
    // E.g. std::bad_alloc -> ENOMEM.
    strncpy(_str, e.what(), _strbufsize);
    _str[_strbufsize - 1] = '\0';
    if (!empty())
    {
        msg::dbg("Exception: %s", _str);
    }
}

bool exc::empty() const throw ()
{
    return (_str[0] == '\0' && _sys_errno == 0);
}

int exc::sys_errno() const throw ()
{
    return _sys_errno;
}

const char *exc::what() const throw ()
{
    return _str;
}

std::ostream &operator<<(std::ostream &os, const exc &e)
{
    os << e.what();
    return os;
}
