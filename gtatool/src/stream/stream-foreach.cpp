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


extern "C" void gtatool_stream_foreach_help(void)
{
    msg::req_txt(
            "stream-foreach [-n|--n=<N>] command [<files...>]\n"
            "\n"
            "Executes the given command for each block of N input GTAs.\n"
            "The command must read N GTAs from its standard input, and must "
            "write any number (including zero) of GTAs to its standard output.\n"
            "The N orginal GTAs are replaced by these new GTAs in the stream.\n"
            "The default is N=1.\n"
            "The special string %%I in the command is replaced by the index of the "
            "current block of GTAs."
            "Example:\n"
            "stream-foreach 'gta tag --set-global=\"X-INDEX=%%I\"' in.gta > numbered.gta");
}

#ifdef HAVE_SIGACTION
static volatile sig_atomic_t sigpipe_flag = 0;
void sigpipe_handler(int /* signum */)
{
    sigpipe_flag = 1;
}
#else
static const int sigpipe_flag = 0;
#endif

extern "C" int gtatool_stream_foreach(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    opt::val<uintmax_t> n("n", 'n', opt::optional, 1, std::numeric_limits<uintmax_t>::max(), 1);
    options.push_back(&n);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 1, -1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_stream_foreach_help();
        return 0;
    }

    if (fio::isatty(gtatool_stdout))
    {
        msg::err_txt("refusing to write to a tty");
        return 1;
    }

#ifdef HAVE_SIGACTION
    struct sigaction new_sigpipe_handler, old_sigpipe_handler;
    new_sigpipe_handler.sa_handler = sigpipe_handler;
    sigemptyset(&new_sigpipe_handler.sa_mask);
    new_sigpipe_handler.sa_flags = 0;
    (void)sigaction(SIGPIPE, &new_sigpipe_handler, &old_sigpipe_handler);
#endif

#if !(W32)
    // Because of the needs of the GUI, we must only write GTAs to gtatool_stdout,
    // so make sure the child process uses that as its standard output.
    // This does not work on Windows, which means we cannot use this
    // command from the GUI on that platform.
    int stdout_bak;
    if ((stdout_bak = dup(1)) < 0 || dup2(fileno(gtatool_stdout), 1) < 0)
    {
        msg::err_txt("cannot set stdout for child process: %s", std::strerror(errno));
        return 1;
    }
#endif

    int retval = 0;
    try
    {
        std::string command = arguments[0];
        arguments.erase(arguments.begin());
        array_loop_t array_loop;
        gta::header hdri, hdro;
        std::string namei, nameo;
        uintmax_t block_index = 0;
        array_loop.start(arguments, "");
        while (array_loop.read(hdri, namei))
        {
            // Open command
            std::string cmd = command;
            str::replace(cmd, "%I", str::from(block_index));
            errno = 0;
            FILE* p = popen(cmd.c_str(), "w");
            if (!p)
            {
                if (errno == 0)
                    errno = ENOMEM;
                throw exc(std::string("cannot run command '") + cmd + "': " + std::strerror(errno));
            }
            // Write N GTAs to command
            try
            {
                uintmax_t i = 0;
                for (;;)
                {
                    hdro = hdri;
                    hdro.set_compression(gta::none);
                    hdro.write_to(p);
                    hdri.copy_data(array_loop.file_in(), hdro, p);
                    i++;
                    if (i >= n.value() || !array_loop.read(hdri, namei))
                        break;
                }
                if (i < n.value())
                {
                    msg::wrn(std::string("last input block only has ") + str::from(i) + " GTAs");
                }
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
                throw exc(std::string("command '") + cmd + "' returned exit status "
                        + str::from(WEXITSTATUS(r)));
            }
            block_index++;
        }
    }
    catch (std::exception &e)
    {
        msg::err_txt("%s", e.what());
        retval = 1;
    }

#if !(W32)
    if (close(1) < 0 || dup2(stdout_bak, 1) < 0 || close(stdout_bak) < 0)
    {
        msg::err_txt("cannot restore stdout: %s", strerror(errno));
        retval = 1;
    }
#endif

#ifdef HAVE_SIGACTION
    (void)sigaction(SIGPIPE, &old_sigpipe_handler, NULL);
#endif

    return retval;
}
