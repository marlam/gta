/*
 * timer.h
 *
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2006, 2007, 2009, 2010
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

#ifndef TIMER_H
#define TIMER_H

#include <ctime>
#include <string>

#if W32
/* Get struct timespec from pthread.h because it is missing from time.h */
# include <pthread.h>
#endif

#include "exc.h"


class Timer
{
private:
    struct timespec _time;

public:
    enum type
    {
        realtime,
        monotonic,
        process_cpu,
        thread_cpu
    };

    Timer() throw ()
    {
        _time.tv_sec = 0;
        _time.tv_nsec = 0;
    }

    Timer(const Timer &t) throw ()
    {
        _time.tv_sec = t._time.tv_sec;
        _time.tv_nsec = t._time.tv_nsec;
    }

    Timer(type t) throw (exc);

    Timer(double seconds) throw ();

    Timer(const std::string &s) throw (exc);

    ~Timer() throw ()
    {
    }

    void set(type t = realtime) throw (exc);

    std::string str() const throw (exc);
    std::string date_str() const throw (exc);

    double seconds() const throw ();

    int compare(const Timer &t) const throw ();

    bool operator==(const Timer &t) const throw ();
    bool operator!=(const Timer &t) const throw ();
    bool operator<(const Timer &t) const throw ();
    bool operator<=(const Timer &t) const throw ();
    bool operator>(const Timer &t) const throw ();
    bool operator>=(const Timer &t) const throw ();

    Timer operator+(const Timer &t) const throw ();
    Timer operator-(const Timer &t) const throw ();
    const Timer &operator=(const Timer &t) throw ();
    const Timer &operator-=(const Timer &t) throw ();
    const Timer &operator+=(const Timer &t) throw ();
};

#endif
