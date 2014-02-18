/*
 * Copyright (C) 2011, 2012, 2013
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
#include <limits>
#include <time.h>

#if HAVE_SYSCONF
# include <unistd.h>
#endif
#if HAVE_SCHED_YIELD
# include <sched.h>
#endif

#if HAVE_SYSCONF && HAVE_SCHED_YIELD
#else
# define WIN32_LEAN_AND_MEAN    /* do not include more than necessary */
# define _WIN32_WINNT 0x0500    /* Windows 2000 or later */
# include <windows.h>
#endif

#include "sys.h"


uintmax_t sys::total_ram()
{
    static uintmax_t s = 0;
    if (s == 0) {
#if HAVE_SYSCONF
        uintmax_t pages = sysconf(_SC_PHYS_PAGES);
        uintmax_t page_size = sysconf(_SC_PAGE_SIZE);
        s = page_size * pages;
#else
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof(statex);
        GlobalMemoryStatusEx(&statex);
        s = statex.ullTotalPhys;
#endif
    }
    return s;
}

int sys::processors()
{
    static long n = -1;
    if (n < 0) {
#if HAVE_SYSCONF
        n = sysconf(_SC_NPROCESSORS_ONLN);
#else
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        n = si.dwNumberOfProcessors;
#endif
        if (n < 1)
            n = 1;
        else if (n > std::numeric_limits<int>::max())
            n = std::numeric_limits<int>::max();
    }
    return n;
}

void sys::msleep(unsigned long msecs)
{
#if HAVE_NANOSLEEP
    struct timespec ts;
    ts.tv_sec = msecs / 1000UL;
    ts.tv_nsec = (msecs - 1000UL * ts.tv_sec) * 1000ULL * 1000ULL;
    nanosleep(&ts, NULL);
#else
    Sleep(msecs);
#endif
}

void sys::usleep(unsigned long usecs)
{
#if HAVE_NANOSLEEP
    struct timespec ts;
    ts.tv_sec = usecs / (1000UL * 1000UL);
    ts.tv_nsec = (usecs - 1000ULL * 1000ULL * ts.tv_sec) * 1000ULL;
    nanosleep(&ts, NULL);
#else
    Sleep(usecs >= 1000UL ? usecs / 1000UL : 1);
#endif
}

void sys::sleep(unsigned long secs)
{
#if HAVE_NANOSLEEP
    struct timespec ts;
    ts.tv_sec = secs;
    ts.tv_nsec = 0;
    nanosleep(&ts, NULL);
#else
    Sleep(secs * 1000UL);
#endif
}

void sys::sched_yield()
{
#if HAVE_SCHED_YIELD
    ::sched_yield();
#else
    SleepEx(0, 0);
#endif
}
