/*
 * main.cpp
 *
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010
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
#include <cstdlib>
#if W32
#   include <fcntl.h>
#endif
#if DYNAMIC_MODULES
#   include <dlfcn.h>
#endif

#include <gta/gta.hpp>

#include "msg.h"
#include "opt.h"
#include "debug.h"

char *program_name;


// The code to manage and start the commands was taken from cvtool-0.2.6.

/*
 * The command functions. All live in their own .cpp file, except for the
 * trivial help and version commands. Some are builtin, and some are loaded
 * as modules (notably those that depend on external libraries).
 * Add the aproppriate lines for each command in this section.
 * The commands must appear in ascending order according to strcmp(), because we
 * do a binary search on the command name.
 */

typedef enum
{
    component,
    dimension,
    array,
    stream,
    conversion,
    misc
} command_category_t;

typedef struct
{
    const char *name;
    command_category_t category;
    void *module_handle;
    int (*cmd)(int argc, char *argv[]);
    void (*cmd_print_help)(void);
} command_t;

#define COMMAND_DECL(FNBASE)      \
    extern "C" int gtatool_ ## FNBASE (int argc, char *argv[]); \
    extern "C" void gtatool_ ## FNBASE ## _help (void);

#define COMMAND_STATIC(NAME, CATEGORY, FNBASE) { NAME, CATEGORY, NULL, gtatool_ ## FNBASE, gtatool_ ## FNBASE ## _help }
#if DYNAMIC_MODULES
#   define COMMAND_MODULE(NAME, CATEGORY, FNBASE) { NAME, CATEGORY, NULL, NULL, NULL }
#else
#   define COMMAND_MODULE(NAME, CATEGORY, FNBASE) COMMAND_STATIC(NAME, CATEGORY, FNBASE)
#endif

COMMAND_DECL(component_add)
COMMAND_DECL(component_compute)
COMMAND_DECL(component_convert)
COMMAND_DECL(component_extract)
COMMAND_DECL(component_merge)
COMMAND_DECL(component_reorder)
COMMAND_DECL(component_set)
COMMAND_DECL(compress)
COMMAND_DECL(create)
COMMAND_DECL(dimension_add)
COMMAND_DECL(dimension_extract)
COMMAND_DECL(dimension_merge)
COMMAND_DECL(dimension_reorder)
COMMAND_DECL(dimension_reverse)
COMMAND_DECL(extract)
COMMAND_DECL(fill)
COMMAND_DECL(from_dcmtk)
COMMAND_DECL(from_exr)
COMMAND_DECL(from_gdal)
COMMAND_DECL(from_magick)
COMMAND_DECL(from_netpbm)
COMMAND_DECL(from_pfs)
COMMAND_DECL(from_raw)
COMMAND_DECL(help)
COMMAND_DECL(info)
COMMAND_DECL(merge)
COMMAND_DECL(resize)
COMMAND_DECL(set)
COMMAND_DECL(tag)
COMMAND_DECL(to_exr)
COMMAND_DECL(to_gdal)
COMMAND_DECL(to_magick)
COMMAND_DECL(to_netpbm)
COMMAND_DECL(to_pfs)
COMMAND_DECL(to_raw)
COMMAND_DECL(uncompress)
COMMAND_DECL(version)

command_t commands[] =
{
    COMMAND_STATIC("component-add",     component,  component_add),
#ifdef HAVE_LIBMUPARSER
    COMMAND_MODULE("component-compute", component,  component_compute),
#endif
    COMMAND_STATIC("component-convert", component,  component_convert),
    COMMAND_STATIC("component-extract", component,  component_extract),
    COMMAND_STATIC("component-merge",   component,  component_merge),
    COMMAND_STATIC("component-reorder", component,  component_reorder),
    COMMAND_STATIC("component-set",     component,  component_set),
    COMMAND_STATIC("compress",          array,      compress),
    COMMAND_STATIC("create",            array,      create),
    COMMAND_STATIC("dimension-add",     dimension,  dimension_add),
    COMMAND_STATIC("dimension-extract", dimension,  dimension_extract),
    COMMAND_STATIC("dimension-merge",   dimension,  dimension_merge),
    COMMAND_STATIC("dimension-reorder", dimension,  dimension_reorder),
    COMMAND_STATIC("dimension-reverse", dimension,  dimension_reverse),
    COMMAND_STATIC("extract",           array,      extract),
    COMMAND_STATIC("fill",              array,      fill),
#ifdef HAVE_LIBDCMIMGLE
    COMMAND_MODULE("from-dcmtk",        conversion, from_dcmtk),
#endif
#ifdef HAVE_LIBILMIMF
    COMMAND_MODULE("from-exr",          conversion, from_exr),
#endif
#ifdef HAVE_LIBGDAL
    COMMAND_MODULE("from-gdal",         conversion, from_gdal),
#endif
#ifdef HAVE_LIBWAND
    COMMAND_MODULE("from-magick",       conversion, from_magick),
#endif
#ifdef HAVE_LIBNETPBM
    COMMAND_MODULE("from-netpbm",       conversion, from_netpbm),
#endif
#ifdef HAVE_LIBPFS_1_2
    COMMAND_MODULE("from-pfs",          conversion, from_pfs),
#endif
    COMMAND_STATIC("from-raw",          conversion, from_raw),
    COMMAND_STATIC("help",              misc,       help),
    COMMAND_STATIC("info",              array,      info),
    COMMAND_STATIC("merge",             array,      merge),
    COMMAND_STATIC("resize",            array,      resize),
    COMMAND_STATIC("set",               array,      set),
    COMMAND_STATIC("tag",               array,      tag),
#ifdef HAVE_LIBILMIMF
    COMMAND_MODULE("to-exr",            conversion, to_exr),
#endif
#ifdef HAVE_LIBGDAL
    COMMAND_MODULE("to-gdal",           conversion, to_gdal),
#endif
#ifdef HAVE_LIBWAND
    COMMAND_MODULE("to-magick",         conversion, to_magick),
#endif
#ifdef HAVE_LIBNETPBM
    COMMAND_MODULE("to-netpbm",         conversion, to_netpbm),
#endif
#ifdef HAVE_LIBPFS_1_2
    COMMAND_MODULE("to-pfs",            conversion, to_pfs),
#endif
    COMMAND_STATIC("to-raw",            conversion, to_raw),
    COMMAND_STATIC("uncompress",        array,      uncompress),
    COMMAND_STATIC("version",           misc,       version),
    { NULL, misc, NULL, NULL, NULL }
};


/*
 * If you just want to add a command, there's no need to change anything below
 * this line.
 */

int cmd_strcmp(const void *a, const void *b)
{
    const command_t *c1 = static_cast<const command_t *>(a);
    const command_t *c2 = static_cast<const command_t *>(b);
    return strcmp(c1->name, c2->name);
}

int cmd_find(const char *cmd)
{
    command_t *p;
    command_t key = { cmd, misc, NULL, NULL, NULL };

    p = static_cast<command_t *>(bsearch(
                static_cast<void *>(&key),
                static_cast<void *>(commands),
                sizeof(commands) / sizeof(commands[0]) - 1,
                sizeof(command_t),
                cmd_strcmp));
    if (!p)
    {
        return -1;
    }
    int cmd_index = p - commands;
#if DYNAMIC_MODULES
    if (!commands[cmd_index].cmd)
    {
        std::string cmd_name = commands[cmd_index].name;
        std::string module_name = std::string(PKGLIBDIR) + "/" + cmd_name + ".so";
        std::string fn_name = std::string("gtatool_") + str::replace(cmd_name, "-", "_");
        std::string help_fn_name = fn_name + "_help";
        commands[cmd_index].module_handle = dlopen(module_name.c_str(), RTLD_LAZY);
        if (!commands[cmd_index].module_handle)
        {
            msg::err("%s", dlerror());
            exit(1);
        }
        commands[cmd_index].cmd = reinterpret_cast<int (*)(int, char **)>(
                dlsym(commands[cmd_index].module_handle, fn_name.c_str()));
        if (!commands[cmd_index].cmd)
        {
            msg::err("%s", dlerror());
            exit(1);
        }
        commands[cmd_index].cmd_print_help = reinterpret_cast<void (*)()>(
                dlsym(commands[cmd_index].module_handle, help_fn_name.c_str()));
        if (!commands[cmd_index].cmd_print_help)
        {
            msg::err("%s", dlerror());
            exit(1);
        }
    }
#endif
    return cmd_index;
}

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
            "Copyright (C) 2010  Martin Lambers <marlam@marlam.de>.\n"
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
                "Usage: %s [-q|--quiet] [-v|--verbose] <command> [argument...]\n",
                program_name);
        msg::req_txt("\nCommands that operate on element component level:");
        for (int i = 0; commands[i].name; i++)
        {
            if (commands[i].category == component)
            {
                msg::req("%s", commands[i].name);
            }
        }
        msg::req_txt("\nCommands that operate on dimension level:");
        for (int i = 0; commands[i].name; i++)
        {
            if (commands[i].category == dimension)
            {
                msg::req("%s", commands[i].name);
            }
        }
        msg::req_txt("\nCommands that operate on array level:");
        for (int i = 0; commands[i].name; i++)
        {
            if (commands[i].category == array)
            {
                msg::req("%s", commands[i].name);
            }
        }
        /*
        msg::req_txt("Commands that operate on stream level:");
        for (int i = 0; commands[i].name; i++)
        {
            if (commands[i].category == stream)
            {
                msg::req("%s", commands[i].name);
            }
        }
        */
        msg::req_txt("\nCommands to convert from/to other file formats:");
        for (int i = 0; commands[i].name; i++)
        {
            if (commands[i].category == conversion)
            {
                msg::req("%s", commands[i].name);
            }
        }
        msg::req_txt("\nMiscellaneous commands:");
        for (int i = 0; commands[i].name; i++)
        {
            if (commands[i].category == misc)
            {
                msg::req("%s", commands[i].name);
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
        else
        {
            commands[cmd_index].cmd_print_help();
#if DYNAMIC_MODULES
            if (commands[cmd_index].module_handle)
            {
                (void)dlclose(commands[cmd_index].module_handle);
            }
#endif
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
#endif
    msg::set_level(msg::INF);
    program_name = strrchr(argv[0], '/');
    program_name = program_name ? program_name + 1 : argv[0];
    msg::set_program_name(program_name);
    msg::set_columns_from_env();
    debug::init_crashhandler();
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
            msg::set_level(msg::WRN);
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
        else
        {
            msg::set_program_name(msg::program_name() + " " + argv[argv_cmd_index]);
            exitcode = commands[cmd_index].cmd(argc - argv_cmd_index, &(argv[argv_cmd_index]));
#if DYNAMIC_MODULES
            if (commands[cmd_index].module_handle)
            {
                (void)dlclose(commands[cmd_index].module_handle);
            }
#endif
        }
    }
    return exitcode;
}
