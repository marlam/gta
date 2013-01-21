/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013
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

#if W32
#   include <stdlib.h>
#   include <io.h>
#   include <fcntl.h>
#   include <string.h>
#   include <strings.h>
#endif

#include <gta/gta.hpp>

#include "msg.h"
#include "opt.h"
#include "dbg.h"

#include "lib.h"
#include "cmds.h"


extern "C" void gtatool_version_help(void)
{
    msg::req_txt(
            "version\n"
            "\n"
            "Print version information.");
}

extern "C" int gtatool_version(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 0, 0, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_version_help();
        return 0;
    }
    msg::req_txt("%s version %s on %s, using libgta version %s\n"
            "Copyright (C) 2013  Martin Lambers <marlam@marlam.de>.\n"
            "This is free software. You may redistribute copies of it under the terms of "
            "the GNU General Public License.\n"
            "There is NO WARRANTY, to the extent permitted by law.",
            PACKAGE_NAME, VERSION, PLATFORM, gta::version());
    return 0;
}

extern "C" void gtatool_help_help(void)
{
    msg::req_txt(
            "help [<command>]\n"
            "\n"
            "Print general or command specific help.");
}

extern "C" int gtatool_help(int argc, char *argv[])
{
    std::vector<opt::option *> options;
    opt::info help("help", '\0', opt::optional);
    options.push_back(&help);
    std::vector<std::string> arguments;
    if (!opt::parse(argc, argv, options, 0, 1, arguments))
    {
        return 1;
    }
    if (help.value())
    {
        gtatool_help_help();
        return 0;
    }
    if (arguments.size() == 0)
    {
        msg::req_txt(
                "Usage: %s [-q|--quiet] [-v|--verbose] <command> [argument...]",
                program_name);
        cmd_category_t categories[] = {
            component,
            dimension,
            array,
            stream,
            conversion,
            misc
        };
        const char *descriptions[] = {
            "Commands that operate on element component level",
            "Commands that operate on dimension level",
            "Commands that operate on array level",
            "Commands that operate on stream level",
            "Commands to convert from/to other file formats",
            "Miscellaneous commands"
        };
        for (size_t i = 0; i < sizeof(categories) / sizeof(categories[0]); i++)
        {
            msg::req_txt("\n%s:", descriptions[i]);
            for (int j = 0; j < cmd_count(); j++)
            {
                if (cmd_category(j) == categories[i])
                {
                    msg::req("%s%s", cmd_name(j), cmd_is_available(j) ? "" : " [unavailable]");
                }
            }
        }
        msg::req_txt(
                "\n"
                "Use \"%s help <command>\" for command specific help.\n"
                "Report bugs to <%s>.", program_name, PACKAGE_BUGREPORT);
        return 0;
    }
    else
    {
        int cmd_index = cmd_find(argv[1]);
        if (cmd_index < 0)
        {
            msg::err("command unknown: %s", argv[1]);
            return 1;
        }
        else if (!cmd_is_available(cmd_index))
        {
            msg::err("command %s is not available in this version of %s", argv[1], PACKAGE_NAME);
            return 1;
        }
        else
        {
            cmd_open(cmd_index);
            cmd_run_help(cmd_index);
            cmd_close(cmd_index);
            return 0;
        }
    }
}

int main(int argc, char *argv[])
{
    int exitcode = 0;
#if W32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _fmode = _O_BINARY;
    setbuf(stderr, NULL);
    program_name = strrchr(argv[0], '\\');
    program_name = program_name ? program_name + 1 : argv[0];
    size_t program_name_len = strlen(program_name);
    if (program_name_len > 4 && strcasecmp(program_name + program_name_len - 4, ".exe") == 0)
    {
        program_name = strdup(program_name);
        program_name[program_name_len - 4] = '\0';
    }
#else
    program_name = strrchr(argv[0], '/');
    program_name = program_name ? program_name + 1 : argv[0];
#endif
    msg::set_level(msg::WRN);
    msg::set_program_name(program_name);
    msg::set_columns_from_env();
    dbg::init_crashhandler();

    if (argc < 2)
    {
        char help[] = "help";
        char *my_argv[] = { help, NULL };
        gtatool_help(1, my_argv);
        exitcode = 1;
    }
    else if (argc == 2 && strcmp(argv[1], "--help") == 0)
    {
        char help[] = "help";
        char *my_argv[] = { help, NULL };
        exitcode = gtatool_help(1, my_argv);
    }
    else if (argc == 2 && strcmp(argv[1], "--version") == 0)
    {
        char version[] = "version";
        char *my_argv[] = { version, NULL };
        exitcode = gtatool_version(1, my_argv);
    }
    else
    {
        int argv_cmd_index = 1;
        if (argc > argv_cmd_index + 1 && (strcmp(argv[argv_cmd_index], "-q") == 0
                    || strcmp(argv[argv_cmd_index], "--quiet") == 0))
        {
            argv_cmd_index++;
            msg::set_level(msg::ERR);
        }
        if (argc > argv_cmd_index + 1 && (strcmp(argv[argv_cmd_index], "-v") == 0
                    || strcmp(argv[argv_cmd_index], "--verbose") == 0))
        {
            argv_cmd_index++;
            msg::set_level(msg::DBG);
        }
        int cmd_index = cmd_find(argv[argv_cmd_index]);
        if (cmd_index < 0)
        {
            msg::err("command unknown: %s", argv[argv_cmd_index]);
            exitcode = 1;
        }
        else if (!cmd_is_available(cmd_index))
        {
            msg::err("command %s is not available in this version of %s", argv[1], PACKAGE_NAME);
            exitcode = 1;
        }
        else
        {
            msg::set_program_name(msg::program_name() + " " + argv[argv_cmd_index]);
            cmd_open(cmd_index);
            gtatool_stdin = stdin;
            gtatool_stdout = stdout;
            exitcode = cmd_run(cmd_index, argc - argv_cmd_index, &(argv[argv_cmd_index]));
            cmd_close(cmd_index);
        }
    }
    return exitcode;
}
