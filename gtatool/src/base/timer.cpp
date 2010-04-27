/*
 * timer.cpp
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
 * timer.cpp
 */

#include "config.h"

#include <sstream>
#include <cerrno>
#include <cmath>

#if W32
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#   include <sys/time.h>
#endif

#include "str.h"
#include "timer.h"


Timer::Timer(Timer::type t) throw (exc)
{
    set(t);
}

Timer::Timer(double seconds) throw ()
{
    if (!std::isfinite(seconds) || seconds <= 0.0)
    {
        _time.tv_sec = 0;
        _time.tv_nsec = 0;
    }
    else
    {
        _time.tv_sec = seconds;
        _time.tv_nsec = (seconds - _time.tv_sec) * 1000000000.0;
    }
}

Timer::Timer(const std::string &s) throw (exc)
{
    size_t dot = s.find('.');
    std::istringstream sec(s.substr(0, dot));
    sec >> _time.tv_sec;
    if (sec.fail())
    {
        throw exc("cannot read time from string", EINVAL);
    }
    if (dot != std::string::npos)
    {
        std::istringstream nsec(s.substr(dot));
        nsec >> _time.tv_nsec;
        if (nsec.fail())
        {
            throw exc("cannot read time from string", EINVAL);
        }
    }
    else
    {
        _time.tv_nsec = 0;
    }
}

void Timer::set(Timer::type t) throw (exc)
{
#if HAVE_CLOCK_GETTIME

    int r = clock_gettime(
              t == realtime ? CLOCK_REALTIME
            : t == monotonic ? CLOCK_MONOTONIC
            : t == process_cpu ? CLOCK_PROCESS_CPUTIME_ID
            : CLOCK_THREAD_CPUTIME_ID, &_time);
    if (r != 0)
    {
        throw exc(std::string("cannot get ")
                + std::string(
                      t == realtime ? "real"
                    : t == monotonic ? "monotonic"
                    : t == process_cpu ? "process CPU"
                    : "thread CPU")
                + std::string("time"), errno);
    }

#else

    // This is only for W32; every sane system has clock_gettime().

    if (t == realtime)
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        _time.tv_sec = tv.tv_sec;
        _time.tv_nsec = tv.tv_usec * 1000;
    }
    else if (t == monotonic)
    {
        LARGE_INTEGER now, frequency;
        QueryPerformanceCounter(&now);
        QueryPerformanceFrequency(&frequency);
        _time.tv_sec = now.QuadPart / frequency.QuadPart;
        _time.tv_sec = ((now.QuadPart - _time.tv_sec) * 1000000000) / frequency.QuadPart;
    }
    else if (t == process_cpu)
    {
        // In W32, clock() starts with zero on program start, so we do not need
        // to subtract a start value.
        clock_t c = clock();
        _time.tv_sec = c / CLOCKS_PER_SEC;
        _time.tv_nsec = ((c - _time.tv_sec * CLOCKS_PER_SEC) * 1000000000) / CLOCKS_PER_SEC;
    }
    else
    {
        throw exc("cannot get thread CPU time", ENOSYS);
    }

#endif
}

std::string Timer::str() const throw (exc)
{
    std::string s(str::str(_time.tv_sec));
    if (_time.tv_nsec != 0)
    {
        std::string nsec_string(str::str(_time.tv_nsec));
        size_t z = nsec_string.find_last_not_of('0');
        if (z != std::string::npos)
        {
            nsec_string.erase(z);
        }
        size_t n =
            (    _time.tv_nsec < 10 ? 8
               : _time.tv_nsec < 100 ? 7
               : _time.tv_nsec < 1000 ? 6
               : _time.tv_nsec < 10000 ? 5
               : _time.tv_nsec < 100000 ? 4
               : _time.tv_nsec < 1000000 ? 3
               : _time.tv_nsec < 10000000 ? 2
               : _time.tv_nsec < 100000000 ? 1
               : 0);
        nsec_string.insert(0, n, '0');
        s += std::string(".") + nsec_string;
    }
    return s;
}

std::string Timer::date_str() const throw (exc)
{
    std::string s(ctime(&_time.tv_sec));
    s.erase(s.end() - 1);
    return s;
}

double Timer::seconds() const throw ()
{
    return static_cast<double>(_time.tv_sec) + static_cast<double>(_time.tv_nsec) / 1000000000.0;
}

int Timer::compare(const Timer &t) const throw ()
{
    if (_time.tv_sec < t._time.tv_sec)
    {
        return -1;
    }
    else if (_time.tv_sec == t._time.tv_sec)
    {
        if (_time.tv_nsec < t._time.tv_nsec)
        {
            return -1;
        }
        else if (_time.tv_nsec == t._time.tv_nsec)
        {
            return 0;
        }
        else
        {
            return +1;
        }
    }
    else
    {
        return +1;
    }
}

bool Timer::operator==(const Timer &t) const throw ()
{
    return compare(t) == 0;
}

bool Timer::operator!=(const Timer &t) const throw ()
{
    return compare(t) != 0;
}

bool Timer::operator<(const Timer &t) const throw ()
{
    return compare(t) < 0;
}

bool Timer::operator<=(const Timer &t) const throw ()
{
    return compare(t) <= 0;
}

bool Timer::operator>(const Timer &t) const throw ()
{
    return compare(t) > 0;
}

bool Timer::operator>=(const Timer &t) const throw ()
{
    return compare(t) >= 0;
}

Timer Timer::operator+(const Timer &t) const throw ()
{
    Timer r;
    r._time.tv_sec = _time.tv_sec + t._time.tv_sec;
    r._time.tv_nsec = _time.tv_nsec + t._time.tv_nsec;
    if (r._time.tv_nsec > 1000000000)
    {
        r._time.tv_sec++;
        r._time.tv_nsec -= 1000000000;
    }
    return r;
}

Timer Timer::operator-(const Timer &t) const throw ()
{
    Timer r;
    if (*this <= t)
    {
        return r;
    }
    else
    {
        if (t._time.tv_nsec > _time.tv_nsec)
        {
            r._time.tv_sec = _time.tv_sec - t._time.tv_sec - 1;
            r._time.tv_nsec = 1000000000 - (t._time.tv_nsec - _time.tv_nsec);
        }
        else
        {
            r._time.tv_sec = _time.tv_sec - t._time.tv_sec;
            r._time.tv_nsec = _time.tv_nsec - t._time.tv_nsec;
        }
    }
    return r;
}

const Timer &Timer::operator=(const Timer &t) throw ()
{
    _time.tv_sec = t._time.tv_sec;
    _time.tv_nsec = t._time.tv_nsec;
    return *this;
}

const Timer &Timer::operator-=(const Timer &t) throw ()
{
    *this = (*this - t);
    return *this;
}

const Timer &Timer::operator+=(const Timer &t) throw ()
{
    *this = (*this + t);
    return *this;
}
