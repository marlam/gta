/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011
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
#if DYNAMIC_MODULES
#   include <dlfcn.h>
#endif

#include "msg.h"
#include "opt.h"
#include "debug.h"

#include "cmds.h"


// The code to manage and start the commands was taken from cvtool-0.2.6.

/*
 * The command functions. All live in their own .cpp file, except for the
 * trivial help and version commands. Some are builtin, and some are loaded
 * as modules (notably those that depend on external libraries).
 * Add the aproppriate lines for each command in this section.
 * The commands must appear in ascending order according to strcmp(), because we
 * do a binary search on the command name.
 */

typedef struct
{
    const char *name;
    cmd_category_t category;
    bool available;
    void *module_handle;
    int (*cmd)(int argc, char *argv[]);
    void (*cmd_print_help)(void);
} cmd_t;

#define CMD_DECL(FNBASE)      \
    extern "C" int gtatool_ ## FNBASE (int argc, char *argv[]); \
    extern "C" void gtatool_ ## FNBASE ## _help (void);

#define CMD_STATIC(NAME, CATEGORY, FNBASE, AVAILABLE) \
    { NAME, CATEGORY, AVAILABLE, NULL, \
        AVAILABLE ? gtatool_ ## FNBASE : NULL, AVAILABLE ? gtatool_ ## FNBASE ## _help : NULL}

#if DYNAMIC_MODULES
#   define CMD_MODULE(NAME, CATEGORY, FNBASE, AVAILABLE) \
        { NAME, CATEGORY, AVAILABLE, NULL, NULL, NULL }
#else
#   define CMD_MODULE(NAME, CATEGORY, FNBASE, AVAILABLE) \
        CMD_STATIC(NAME, CATEGORY, FNBASE, AVAILABLE)
#endif

CMD_DECL(component_add)
CMD_DECL(component_compute)
CMD_DECL(component_convert)
CMD_DECL(component_extract)
CMD_DECL(component_merge)
CMD_DECL(component_reorder)
CMD_DECL(component_set)
CMD_DECL(component_split)
CMD_DECL(compress)
CMD_DECL(create)
CMD_DECL(dimension_add)
CMD_DECL(dimension_extract)
CMD_DECL(dimension_merge)
CMD_DECL(dimension_reorder)
CMD_DECL(dimension_reverse)
CMD_DECL(dimension_split)
CMD_DECL(extract)
CMD_DECL(fill)
CMD_DECL(from_dcmtk)
CMD_DECL(from_exr)
CMD_DECL(from_gdal)
CMD_DECL(from_magick)
CMD_DECL(from_mat)
CMD_DECL(from_netpbm)
CMD_DECL(from_pfs)
CMD_DECL(from_rat)
CMD_DECL(from_raw)
CMD_DECL(gui)
CMD_DECL(help)
CMD_DECL(info)
CMD_DECL(merge)
CMD_DECL(resize)
CMD_DECL(set)
CMD_DECL(stream_extract)
CMD_DECL(stream_merge)
CMD_DECL(stream_split)
CMD_DECL(tag)
CMD_DECL(to_exr)
CMD_DECL(to_gdal)
CMD_DECL(to_magick)
CMD_DECL(to_mat)
CMD_DECL(to_netpbm)
CMD_DECL(to_pfs)
CMD_DECL(to_rat)
CMD_DECL(to_raw)
CMD_DECL(uncompress)
CMD_DECL(version)

static cmd_t cmds[] =
{
    CMD_STATIC("component-add",     component,  component_add,     true),
    CMD_MODULE("component-compute", component,  component_compute, WITH_MUPARSER),
    CMD_STATIC("component-convert", component,  component_convert, true),
    CMD_STATIC("component-extract", component,  component_extract, true),
    CMD_STATIC("component-merge",   component,  component_merge,   true),
    CMD_STATIC("component-reorder", component,  component_reorder, true),
    CMD_STATIC("component-set",     component,  component_set,     true),
    CMD_STATIC("component-split",   component,  component_split,   true),
    CMD_STATIC("compress",          array,      compress,          true),
    CMD_STATIC("create",            array,      create,            true),
    CMD_STATIC("dimension-add",     dimension,  dimension_add,     true),
    CMD_STATIC("dimension-extract", dimension,  dimension_extract, true),
    CMD_STATIC("dimension-merge",   dimension,  dimension_merge,   true),
    CMD_STATIC("dimension-reorder", dimension,  dimension_reorder, true),
    CMD_STATIC("dimension-reverse", dimension,  dimension_reverse, true),
    CMD_STATIC("dimension-split",   dimension,  dimension_split,   true),
    CMD_STATIC("extract",           array,      extract,           true),
    CMD_STATIC("fill",              array,      fill,              true),
    CMD_MODULE("from-dcmtk",        conversion, from_dcmtk,        WITH_DCMTK),
    CMD_MODULE("from-exr",          conversion, from_exr,          WITH_EXR),
    CMD_MODULE("from-gdal",         conversion, from_gdal,         WITH_GDAL),
    CMD_MODULE("from-magick",       conversion, from_magick,       WITH_MAGICK),
    CMD_MODULE("from-mat",          conversion, from_mat,          WITH_MAT),
    CMD_MODULE("from-netpbm",       conversion, from_netpbm,       WITH_NETPBM),
    CMD_MODULE("from-pfs",          conversion, from_pfs,          WITH_PFS),
    CMD_STATIC("from-rat",          conversion, from_rat,          true),
    CMD_STATIC("from-raw",          conversion, from_raw,          true),
    CMD_MODULE("gui",               misc,       gui,               WITH_QT),
    CMD_STATIC("help",              misc,       help,              true),
    CMD_STATIC("info",              array,      info,              true),
    CMD_STATIC("merge",             array,      merge,             true),
    CMD_STATIC("resize",            array,      resize,            true),
    CMD_STATIC("set",               array,      set,               true),
    CMD_STATIC("stream-extract",    stream,     stream_extract,    true),
    CMD_STATIC("stream-merge",      stream,     stream_merge,      true),
    CMD_STATIC("stream-split",      stream,     stream_split,      true),
    CMD_STATIC("tag",               array,      tag,               true),
    CMD_MODULE("to-exr",            conversion, to_exr,            WITH_EXR),
    CMD_MODULE("to-gdal",           conversion, to_gdal,           WITH_GDAL),
    CMD_MODULE("to-magick",         conversion, to_magick,         WITH_MAGICK),
    CMD_MODULE("to-mat",            conversion, to_mat,            WITH_MAT),
    CMD_MODULE("to-netpbm",         conversion, to_netpbm,         WITH_NETPBM),
    CMD_MODULE("to-pfs",            conversion, to_pfs,            WITH_PFS),
    CMD_STATIC("to-rat",            conversion, to_rat,            true),
    CMD_STATIC("to-raw",            conversion, to_raw,            true),
    CMD_STATIC("uncompress",        array,      uncompress,        true),
    CMD_STATIC("version",           misc,       version,           true),
};


/*
 * If you just want to add a command, there's no need to change anything below
 * this line.
 */

int cmd_count()
{
    return (sizeof(cmds) / sizeof(cmds[0]));
}

const char *cmd_name(int cmd_index)
{
    return cmds[cmd_index].name;
}

cmd_category_t cmd_category(int cmd_index)
{
    return cmds[cmd_index].category;
}

bool cmd_is_available(int cmd_index)
{
    return cmds[cmd_index].available;
}

static int cmd_strcmp(const void *a, const void *b)
{
    const cmd_t *c1 = static_cast<const cmd_t *>(a);
    const cmd_t *c2 = static_cast<const cmd_t *>(b);
    return strcmp(c1->name, c2->name);
}

int cmd_find(const char *cmd)
{
    cmd_t *p;
    cmd_t key = { cmd, misc, false, NULL, NULL, NULL };

    p = static_cast<cmd_t *>(bsearch(
                static_cast<void *>(&key),
                static_cast<void *>(cmds),
                cmd_count(),
                sizeof(cmd_t),
                cmd_strcmp));
    int cmd_index = (p ? p - cmds : -1);
    return cmd_index;
}

void cmd_open(int cmd_index)
{
#if DYNAMIC_MODULES
    if (cmds[cmd_index].available && !cmds[cmd_index].cmd)
    {
        std::string cmd_name = cmds[cmd_index].name;
        std::string module_name = std::string(PKGLIBDIR) + "/" + cmd_name + ".so";
        std::string fn_name = std::string("gtatool_") + str::replace(cmd_name, "-", "_");
        std::string help_fn_name = fn_name + "_help";
        cmds[cmd_index].module_handle = dlopen(module_name.c_str(), RTLD_LAZY);
        if (!cmds[cmd_index].module_handle)
        {
            msg::err("%s", dlerror());
            exit(1);
        }
        cmds[cmd_index].cmd = reinterpret_cast<int (*)(int, char **)>(
                dlsym(cmds[cmd_index].module_handle, fn_name.c_str()));
        if (!cmds[cmd_index].cmd)
        {
            msg::err("%s", dlerror());
            exit(1);
        }
        cmds[cmd_index].cmd_print_help = reinterpret_cast<void (*)()>(
                dlsym(cmds[cmd_index].module_handle, help_fn_name.c_str()));
        if (!cmds[cmd_index].cmd_print_help)
        {
            msg::err("%s", dlerror());
            exit(1);
        }
    }
#endif
}

void cmd_run_help(int cmd_index)
{
    cmds[cmd_index].cmd_print_help();
}

int cmd_run(int cmd_index, int argc, char *argv[])
{
    return cmds[cmd_index].cmd(argc, argv);
}

void cmd_close(int cmd_index)
{
#if DYNAMIC_MODULES
    if (cmds[cmd_index].module_handle)
    {
        (void)dlclose(cmds[cmd_index].module_handle);
        cmds[cmd_index].module_handle = NULL;
        cmds[cmd_index].cmd_print_help = NULL;
        cmds[cmd_index].cmd = NULL;
    }
#endif
}
