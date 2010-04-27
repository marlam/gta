/*
 * tools.cpp
 *
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2009, 2010  Martin Lambers <marlam@marlam.de>
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
#include <unistd.h>
#if W32
# define WIN32_LEAN_AND_MEAN    /* do not include more than necessary */
# define _WIN32_WINNT 0x0502    /* Windows XP SP2 or later */
# include <windows.h>
# include <winsock2.h>
#else
# include <pwd.h>
# include <time.h>
#endif

#include "str.h"
#include "timer.h"
#include "tools.h"


std::string tools::homedir()
{
#if W32

    char *home;
    BYTE homebuf[MAX_PATH + 1];
    HKEY hkey;
    DWORD len;
    DWORD type;
    long l;

    if (!(home = getenv("HOME")))
    {
        l = RegOpenKeyEx(HKEY_CURRENT_USER,
                "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\"
                "Shell Folders", 0, KEY_READ, &hkey);
        if (l == ERROR_SUCCESS)
        {
            len = MAX_PATH;
            l = RegQueryValueEx(hkey, "AppData", NULL, &type, homebuf, &len);
            if (l == ERROR_SUCCESS && len < MAX_PATH)
            {
                RegCloseKey(hkey);
                home = reinterpret_cast<char *>(homebuf);
            }
        }
    }
    return std::string(home ? home : "C:");

#else

    char *home;
    struct passwd *pw;

    if (!(home = getenv("HOME")))
    {
        pw = getpwuid(getuid());
        if (pw && pw->pw_dir)
        {
            home = pw->pw_dir;
        }
    }

    return std::string(home ? home : "");

#endif
}

std::string tools::app_id(const std::string &appname)
{
    // Use an ID generation method that is similar to the guidelines
    // for maildir clients. See http://cr.yp.to/proto/maildir.html .

    Timer t(Timer::realtime);
    pid_t pid = getpid();

    /* Get a sane hostname */
    char hostname_buf[256];
    if (gethostname(hostname_buf, 256) != 0)
    {
        // Should never happen on any sane system
        strcpy(hostname_buf, "unknown");
    }
    else
    {
        // Make sure the hostname is NUL-terminated
        hostname_buf[255] = '\0';
    }
    std::string hostname(hostname_buf);
    str::replace(hostname, "/", "_057_");
    str::replace(hostname, ":", "_072_");
    str::replace(hostname, "\\", "_134_");

    return (appname.empty() ? std::string("") : (appname + "-"))
        + hostname + std::string("-")
        + str::str(pid) + std::string("-")
        + t.str();
}

void tools::msleep(const unsigned int milliseconds)
{
#if W32
    Sleep(milliseconds);
#else
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds - 1000 * ts.tv_sec) * 1000 * 1000;
    nanosleep(&ts, NULL);
#endif
}
