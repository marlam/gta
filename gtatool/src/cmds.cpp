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
#include "str.h"

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
    const char *module_name;
    void *module_handle;
    int (*cmd)(int argc, char *argv[]);
    void (*cmd_print_help)(void);
} cmd_t;

#define CMD_DECL(FNBASE)      \
    extern "C" int gtatool_ ## FNBASE (int argc, char *argv[]); \
    extern "C" void gtatool_ ## FNBASE ## _help (void);

#define BUILTIN NULL

#if DYNAMIC_MODULES
#   define CMD(NAME, CATEGORY, FNBASE, AVAILABLE, MODULE) \
        { NAME, CATEGORY, AVAILABLE, MODULE, NULL, \
            AVAILABLE && !MODULE ? gtatool_ ## FNBASE : NULL, \
            AVAILABLE && !MODULE ? gtatool_ ## FNBASE ## _help : NULL }
#else
#   define CMD(NAME, CATEGORY, FNBASE, AVAILABLE, MODULE) \
        { NAME, CATEGORY, AVAILABLE, BUILTIN, NULL, \
            AVAILABLE ? gtatool_ ## FNBASE : NULL, \
            AVAILABLE ? gtatool_ ## FNBASE ## _help : NULL }
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
CMD_DECL(dimension_flatten)
CMD_DECL(dimension_merge)
CMD_DECL(dimension_reorder)
CMD_DECL(dimension_reverse)
CMD_DECL(dimension_split)
CMD_DECL(extract)
CMD_DECL(fill)
CMD_DECL(from_dcmtk)
CMD_DECL(from_exr)
CMD_DECL(from_ffmpeg)
CMD_DECL(from_gdal)
CMD_DECL(from_magick)
CMD_DECL(from_mat)
CMD_DECL(from_netpbm)
CMD_DECL(from_pfs)
CMD_DECL(from_ply)
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
CMD_DECL(to_ply)
CMD_DECL(to_rat)
CMD_DECL(to_raw)
CMD_DECL(uncompress)
CMD_DECL(version)

static cmd_t cmds[] =
{
    CMD("component-add",     component,  component_add,     true,          BUILTIN),
    CMD("component-compute", component,  component_compute, WITH_MUPARSER, "component-compute"),
    CMD("component-convert", component,  component_convert, true,          BUILTIN),
    CMD("component-extract", component,  component_extract, true,          BUILTIN),
    CMD("component-merge",   component,  component_merge,   true,          BUILTIN),
    CMD("component-reorder", component,  component_reorder, true,          BUILTIN),
    CMD("component-set",     component,  component_set,     true,          BUILTIN),
    CMD("component-split",   component,  component_split,   true,          BUILTIN),
    CMD("compress",          array,      compress,          true,          BUILTIN),
    CMD("create",            array,      create,            true,          BUILTIN),
    CMD("dimension-add",     dimension,  dimension_add,     true,          BUILTIN),
    CMD("dimension-extract", dimension,  dimension_extract, true,          BUILTIN),
    CMD("dimension-flatten", dimension,  dimension_flatten, true,          BUILTIN),
    CMD("dimension-merge",   dimension,  dimension_merge,   true,          BUILTIN),
    CMD("dimension-reorder", dimension,  dimension_reorder, true,          BUILTIN),
    CMD("dimension-reverse", dimension,  dimension_reverse, true,          BUILTIN),
    CMD("dimension-split",   dimension,  dimension_split,   true,          BUILTIN),
    CMD("extract",           array,      extract,           true,          BUILTIN),
    CMD("fill",              array,      fill,              true,          BUILTIN),
    CMD("from-dcmtk",        conversion, from_dcmtk,        WITH_DCMTK,    "conv-dcmtk"),
    CMD("from-exr",          conversion, from_exr,          WITH_EXR,      "conv-exr"),
    CMD("from-ffmpeg",       conversion, from_ffmpeg,       WITH_FFMPEG,   "conv-ffmpeg"),
    CMD("from-gdal",         conversion, from_gdal,         WITH_GDAL,     "conv-gdal"),
    CMD("from-magick",       conversion, from_magick,       WITH_MAGICK,   "conv-magick"),
    CMD("from-mat",          conversion, from_mat,          WITH_MAT,      "conv-mat"),
    CMD("from-netpbm",       conversion, from_netpbm,       WITH_NETPBM,   "conv-netpbm"),
    CMD("from-pfs",          conversion, from_pfs,          WITH_PFS,      "conv-pfs"),
    CMD("from-ply",          conversion, from_ply,          true,          "conv-ply"),
    CMD("from-rat",          conversion, from_rat,          true,          "conv-rat"),
    CMD("from-raw",          conversion, from_raw,          true,          "conv-raw"),
    CMD("gui",               misc,       gui,               WITH_QT,       "gui"),
    CMD("help",              misc,       help,              true,          BUILTIN),
    CMD("info",              array,      info,              true,          BUILTIN),
    CMD("merge",             array,      merge,             true,          BUILTIN),
    CMD("resize",            array,      resize,            true,          BUILTIN),
    CMD("set",               array,      set,               true,          BUILTIN),
    CMD("stream-extract",    stream,     stream_extract,    true,          BUILTIN),
    CMD("stream-merge",      stream,     stream_merge,      true,          BUILTIN),
    CMD("stream-split",      stream,     stream_split,      true,          BUILTIN),
    CMD("tag",               array,      tag,               true,          BUILTIN),
    CMD("to-exr",            conversion, to_exr,            WITH_EXR,      "conv-exr"),
    CMD("to-gdal",           conversion, to_gdal,           WITH_GDAL,     "conv-gdal"),
    CMD("to-magick",         conversion, to_magick,         WITH_MAGICK,   "conv-magick"),
    CMD("to-mat",            conversion, to_mat,            WITH_MAT,      "conv-mat"),
    CMD("to-netpbm",         conversion, to_netpbm,         WITH_NETPBM,   "conv-netpbm"),
    CMD("to-pfs",            conversion, to_pfs,            WITH_PFS,      "conv-pfs"),
    CMD("to-ply",            conversion, to_ply,            true,          "conv-ply"),
    CMD("to-rat",            conversion, to_rat,            true,          "conv-rat"),
    CMD("to-raw",            conversion, to_raw,            true,          "conv-raw"),
    CMD("uncompress",        array,      uncompress,        true,          BUILTIN),
    CMD("version",           misc,       version,           true,          BUILTIN),
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
    cmd_t key = { cmd, misc, false, NULL, NULL, NULL, NULL };

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
        std::string module_name = std::string(PKGLIBDIR) + "/" + cmds[cmd_index].module_name + ".so";
        std::string fn_name = std::string("gtatool_") + str::replace(cmd_name, "-", "_");
        std::string help_fn_name = fn_name + "_help";
        // We used RTLD_LAZY here, but that broke the from-dcmtk command whith dmctk 3.6.0.
        cmds[cmd_index].module_handle = dlopen(module_name.c_str(), RTLD_NOW);
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
