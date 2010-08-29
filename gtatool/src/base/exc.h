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

/**
 * \file exc.h
 *
 * Error and exception handling.
 */

#ifndef EXC_H
#define EXC_H

#include <exception>
#include <string>

/**
 * Error and exception handling.
 */

class exc : public std::exception
{
    private:
        static const size_t _strbufsize = 256;
        char _str[_strbufsize];
        int _sys_errno;

        void create(const char *when, int sys_errno, const char *what) throw ();

    public:
        exc() throw ();
        exc(const std::string &when, int sys_errno = 0, const std::string &what = std::string()) throw ();
        exc(const std::string &when, const std::string &what) throw ();
        exc(int sys_errno) throw ();
        exc(const exc &e) throw ();
        exc(const std::exception &e) throw ();

        bool empty() const throw ();
        int sys_errno() const throw ();
        virtual const char *what() const throw ();
};

#endif
