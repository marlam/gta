/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2013
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

#include <cstdio>
#include <cerrno>
#include <limits>
#include <unistd.h>
#ifdef HAVE_SIGACTION
# include <signal.h>
#endif
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#else // probably only Windows
# define WIFEXITED(s) (1)
# define WEXITSTATUS(s) (s)
#endif

#include <gta/gta.hpp>

#include "msg.h"
#include "opt.h"
#include "str.h"
#include "fio.h"

#include "lib.h"


extern "C" void gtatool_stream_grep_help(void)
{
    msg::req_txt(
            "stream-grep command [<files...>]\n"
            "\n"
            "Executes the given command for each input GTAs, and outputs only those GTAs "
            "for which the command exits successfully.\n"
            "This can be used to extract GTAs that match certain characteristics from an "
            "input stream.\n"
            "The command must read one GTA from standard input and then exit with zero "
            "(success; the GTA passes) or non-zero (failure; the GTA is removed). Any "
            "output of the command is ignored.\n"
            "Examples:\n"
            "stream-grep 'gta tag --get-global=X-INDEX 2>&1 > /dev/null | grep X-INDEX=8' all.gta > only-8.gta\n"
            "stream-grep 'gta info 2>&1 > /dev/null | grep \"dimension 0: 42\"' all.gta > only-width42.gta");
}

#ifdef HAVE_SIGACTION
static volatile sig_atomic_t sigpipe_flag = 0;
static void sigpipe_handler(int /* signum */)
{
    sigpipe_flag = 1;
}
#else
static const int sigpipe_flag = 0;
#endif

extern "C" int gtatool_stream_grep(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_stream_grep_help();
        return 0;
    }

#ifdef HAVE_SIGACTION
    struct sigaction new_sigpipe_handler, old_sigpipe_handler;
    new_sigpipe_handler.sa_handler = sigpipe_handler;
    sigemptyset(&new_sigpipe_handler.sa_mask);
    new_sigpipe_handler.sa_flags = 0;
    (void)sigaction(SIGPIPE, &new_sigpipe_handler, &old_sigpipe_handler);
#endif

#if W32
    FILE *fdevnull = fio::open("NUL", "w");
#else
    FILE *fdevnull = fio::open("/dev/null", "w");
#endif

    FILE *tmpf = NULL;
    int retval = 0;
    try
    {
        std::string command = arguments[0];
        arguments.erase(arguments.begin());
        array_loop_t array_loop;
        gta::header hdri, hdro;
        std::string namei, nameo;
        array_loop.start(arguments, "");
        while (array_loop.read(hdri, namei))
        {
            // Make sure the output of the child process is ignored.
            int stdout_bak = -1;
            if ((stdout_bak = dup(1)) < 0 || dup2(fileno(fdevnull), 1) < 0)
            {
                throw exc(std::string("cannot set stdout for child process: ") + std::strerror(errno));
            }
            // Open command
            std::string cmd = command;
            errno = 0;
            FILE* p = popen(cmd.c_str(), "w");
            if (!p)
            {
                if (errno == 0)
                    errno = ENOMEM;
                throw exc(std::string("cannot run command '") + cmd + "': " + std::strerror(errno));
            }
            // Buffer GTA data to temp file
            hdro = hdri;
            hdro.set_compression(gta::none);
            tmpf = fio::tempfile();
            hdri.copy_data(array_loop.file_in(), hdro, tmpf);
            fio::rewind(tmpf);
            // Write 1 GTA to command
            try
            {
                hdro.write_to(p);
                hdro.copy_data(tmpf, hdro, p);
            }
            catch (gta::exception& e)
            {
                if (sigpipe_flag && e.result() == gta::system_error && e.sys_errno() == EPIPE)
                {
                    // ignore: we got sigpipe and handle that below
                }
                else
                {
                    (void)pclose(p);
                    throw e;
                }
            }
            catch (...)
            {
                (void)pclose(p);
                throw;
            }
            // Close command
            bool keep_gta = true;
            int r = pclose(p);
            if (r == -1 || !WIFEXITED(r) || WEXITSTATUS(r) == 127)
            {
                throw exc(std::string("command '") + cmd + "' failed to execute");
            }
            else if (sigpipe_flag)
            {
                throw exc(std::string("command '") + cmd + "' did not read its stdin");
            }
            else if (WEXITSTATUS(r) != 0)
            {
                keep_gta = false;
            }
            // Restore stdout
            if (close(1) < 0 || dup2(stdout_bak, 1) < 0 || close(stdout_bak) < 0)
            {
                throw exc(std::string("cannot restore stdout: ") + strerror(errno));
            }
            // Write array
            if (keep_gta)
            {
                array_loop.write(hdro, nameo);
                fio::rewind(tmpf);
                hdro.copy_data(tmpf, hdro, array_loop.file_out());
            }
            fio::close(tmpf);
            tmpf = NULL;
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        retval = 1;
    }
    fclose(fdevnull);
    if (tmpf)
    {
        fclose(tmpf);
    }

#ifdef HAVE_SIGACTION
    (void)sigaction(SIGPIPE, &old_sigpipe_handler, NULL);
#endif

    return retval;
}
