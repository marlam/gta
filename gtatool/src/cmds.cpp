/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
 * 2015, 2016
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

#include "base/msg.h"
#include "base/opt.h"
#include "base/str.h"
#include "base/fio.h"

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
    const char* description;
} cmd_t;

#define CMD_DECL(FNBASE)      \
    extern "C" int gtatool_ ## FNBASE (int argc, char *argv[]); \
    extern "C" void gtatool_ ## FNBASE ## _help (void);

#define BUILTIN NULL

#if DYNAMIC_MODULES
#   define CMD(NAME, CATEGORY, FNBASE, AVAILABLE, MODULE, DESCR) \
        { NAME, CATEGORY, AVAILABLE, MODULE, NULL, \
            AVAILABLE && (MODULE == NULL) ? gtatool_ ## FNBASE : NULL, \
            AVAILABLE && (MODULE == NULL) ? gtatool_ ## FNBASE ## _help : NULL, \
            DESCR }
#else
#   define CMD(NAME, CATEGORY, FNBASE, AVAILABLE, MODULE, DESCR) \
        { NAME, CATEGORY, AVAILABLE, BUILTIN, NULL, \
            AVAILABLE ? gtatool_ ## FNBASE : NULL, \
            AVAILABLE ? gtatool_ ## FNBASE ## _help : NULL, \
            DESCR }
#endif

CMD_DECL(combine)
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
CMD_DECL(diff)
CMD_DECL(dimension_add)
CMD_DECL(dimension_extract)
CMD_DECL(dimension_flatten)
CMD_DECL(dimension_merge)
CMD_DECL(dimension_reorder)
CMD_DECL(dimension_reverse)
CMD_DECL(dimension_split)
CMD_DECL(extract)
CMD_DECL(fill)
CMD_DECL(from)
CMD_DECL(from_csv)
CMD_DECL(from_datraw)
CMD_DECL(from_dcmtk)
CMD_DECL(from_exr)
CMD_DECL(from_ffmpeg)
CMD_DECL(from_gdal)
CMD_DECL(from_jpeg)
CMD_DECL(from_magick)
CMD_DECL(from_mat)
CMD_DECL(from_netcdf)
CMD_DECL(from_netpbm)
CMD_DECL(from_pcd)
CMD_DECL(from_pfs)
CMD_DECL(from_ply)
CMD_DECL(from_pmd)
CMD_DECL(from_png)
CMD_DECL(from_pvm)
CMD_DECL(from_rat)
CMD_DECL(from_raw)
CMD_DECL(from_sndfile)
CMD_DECL(from_teem)
CMD_DECL(gui)
CMD_DECL(help)
CMD_DECL(info)
CMD_DECL(merge)
CMD_DECL(resize)
CMD_DECL(set)
CMD_DECL(stream_extract)
CMD_DECL(stream_foreach)
CMD_DECL(stream_grep)
CMD_DECL(stream_merge)
CMD_DECL(stream_split)
CMD_DECL(tag)
CMD_DECL(to)
CMD_DECL(to_csv)
CMD_DECL(to_datraw)
CMD_DECL(to_exr)
CMD_DECL(to_gdal)
CMD_DECL(to_jpeg)
CMD_DECL(to_magick)
CMD_DECL(to_mat)
CMD_DECL(to_netcdf)
CMD_DECL(to_netpbm)
CMD_DECL(to_pcd)
CMD_DECL(to_pfs)
CMD_DECL(to_ply)
CMD_DECL(to_png)
CMD_DECL(to_pvm)
CMD_DECL(to_rat)
CMD_DECL(to_raw)
CMD_DECL(to_sndfile)
CMD_DECL(to_teem)
CMD_DECL(uncompress)
CMD_DECL(version)

static cmd_t cmds[] =
{
    CMD("combine",           cmd_array,      combine,           true,          BUILTIN,
            "Combine arrays values (e.g. with min,max,add,mul,...)"),
    CMD("component-add",     cmd_component,  component_add,     true,          BUILTIN,
            "Add components to arrays (e.g. add A to RGB)"),
    CMD("component-compute", cmd_component,  component_compute, WITH_MUPARSER, "component-compute",
            "Compute array component values"),
    CMD("component-convert", cmd_component,  component_convert, true,          BUILTIN,
            "Convert array component types (e.g. uint8 to float32)"),
    CMD("component-extract", cmd_component,  component_extract, true,          BUILTIN,
            "Extract components from arrays (e.g. R from RGB)"),
    CMD("component-merge",   cmd_component,  component_merge,   true,          BUILTIN,
            "Merge arrays on component level (e.g. R,G,B to RGB)"),
    CMD("component-reorder", cmd_component,  component_reorder, true,          BUILTIN,
            "Reorder array components (e.g. RGB to BGR)"),
    CMD("component-set",     cmd_component,  component_set,     true,          BUILTIN,
            "Set array component values"),
    CMD("component-split",   cmd_component,  component_split,   true,          BUILTIN,
            "Split arrays along components (e.g. RGB to R,G,B)"),
    CMD("compress",          cmd_array,      compress,          true,          BUILTIN,
            "Compress arrays"),
    CMD("create",            cmd_array,      create,            true,          BUILTIN,
            "Create arrays"),
    CMD("diff",              cmd_array,      diff,              true,          BUILTIN,
            "Compute differences between arrays"),
    CMD("dimension-add",     cmd_dimension,  dimension_add,     true,          BUILTIN,
            "Add dimensions to arrays (e.g. slice to flat volume)"),
    CMD("dimension-extract", cmd_dimension,  dimension_extract, true,          BUILTIN,
            "Extract dimension from arrays (e.g. slice from volume)"),
    CMD("dimension-flatten", cmd_dimension,  dimension_flatten, true,          BUILTIN,
            "Make arrays one-dimensional"),
    CMD("dimension-merge",   cmd_dimension,  dimension_merge,   true,          BUILTIN,
            "Merge arrays along dimension (e.g. slices to volume)"),
    CMD("dimension-reorder", cmd_dimension,  dimension_reorder, true,          BUILTIN,
            "Reorder array dimensions (e.g. transpose matrix)"),
    CMD("dimension-reverse", cmd_dimension,  dimension_reverse, true,          BUILTIN,
            "Reverse array dimension (e.g. flip image)"),
    CMD("dimension-split",   cmd_dimension,  dimension_split,   true,          BUILTIN,
            "Split arrays along dimension (e.g. volume to slices)"),
    CMD("extract",           cmd_array,      extract,           true,          BUILTIN,
            "Extract parts of arrays"),
    CMD("fill",              cmd_array,      fill,              true,          BUILTIN,
            "Fill parts of arrays"),
    CMD("from",              cmd_conversion, from,              true,          BUILTIN,
            "Import arrays, autodetect input format"),
    CMD("from-csv",          cmd_conversion, from_csv,          WITH_CSV,      "conv-csv",
            "Import arrays from CSV files"),
    CMD("from-datraw",       cmd_conversion, from_datraw,       WITH_DATRAW,   "conv-datraw",
            "Import arrays from .dat/.raw volume data"),
    CMD("from-dcmtk",        cmd_conversion, from_dcmtk,        WITH_DCMTK,    "conv-dcmtk",
            "Import arrays from DICOM data"),
    CMD("from-exr",          cmd_conversion, from_exr,          WITH_EXR,      "conv-exr",
            "Import arrays from OpenEXR files"),
    CMD("from-ffmpeg",       cmd_conversion, from_ffmpeg,       WITH_FFMPEG,   "conv-ffmpeg",
            "Import arrays from multimedia files via FFmpeg"),
    CMD("from-gdal",         cmd_conversion, from_gdal,         WITH_GDAL,     "conv-gdal",
            "Import arrays from remote sensing data via GDAL"),
    CMD("from-jpeg",         cmd_conversion, from_jpeg,         WITH_JPEG,     "conv-jpeg",
            "Import arrays from JPEG images"),
    CMD("from-magick",       cmd_conversion, from_magick,       WITH_MAGICK,   "conv-magick",
            "Import arrays from images via " MAGICK_FLAVOR),
    CMD("from-mat",          cmd_conversion, from_mat,          WITH_MAT,      "conv-mat",
            "Import arrays from Matlab files"),
    CMD("from-netcdf",       cmd_conversion, from_netcdf,       WITH_NETCDF,   "conv-netcdf",
            "Import arrays from NetCDF files (incl. HDF4/5)"),
    CMD("from-netpbm",       cmd_conversion, from_netpbm,       WITH_NETPBM,   "conv-netpbm",
            "Import arrays from NetPBM images"),
    CMD("from-pcd",          cmd_conversion, from_pcd,          WITH_PCD,      "conv-pcd",
            "Import arrays from PCD point cloud data"),
    CMD("from-pfs",          cmd_conversion, from_pfs,          WITH_PFS,      "conv-pfs",
            "Import arrays from PFS images"),
    CMD("from-ply",          cmd_conversion, from_ply,          WITH_PLY,      "conv-ply",
            "Import arrays from PLY point data"),
    CMD("from-pmd",          cmd_conversion, from_pmd,          WITH_PMD,      "conv-pmd",
            "Import arrays from PMD ToF sensor data files"),
    CMD("from-png",          cmd_conversion, from_png,          WITH_PNG,      "conv-png",
            "Import arrays from PNG files"),
    CMD("from-pvm",          cmd_conversion, from_pvm,          WITH_PVM,      "conv-pvm",
            "Import arrays from PVM volume data"),
    CMD("from-rat",          cmd_conversion, from_rat,          WITH_RAT,      "conv-rat",
            "Import arrays from RAT RadarTools data"),
    CMD("from-raw",          cmd_conversion, from_raw,          WITH_RAW,      "conv-raw",
            "Import arrays from raw binary data"),
    CMD("from-sndfile",      cmd_conversion, from_sndfile,      WITH_SNDFILE,  "conv-sndfile",
            "Import arrays from audio files via libsndfile"),
    CMD("from-teem",         cmd_conversion, from_teem,         WITH_TEEM,     "conv-teem",
            "Import arrays from NRRD files via Teem"),
    CMD("gui",               cmd_misc,       gui,               WITH_QT,       "gui",
            "Graphical user interface"),
    CMD("help",              cmd_misc,       help,              true,          BUILTIN,
            "Show help"),
    CMD("info",              cmd_array,      info,              true,          BUILTIN,
            "Show information about arrays"),
    CMD("merge",             cmd_array,      merge,             true,          BUILTIN,
            "Merge arrays into larger arrays"),
    CMD("resize",            cmd_array,      resize,            true,          BUILTIN,
            "Resize arrays"),
    CMD("set",               cmd_array,      set,               true,          BUILTIN,
            "Set parts of arrays"),
    CMD("stream-extract",    cmd_stream,     stream_extract,    true,          BUILTIN,
            "Extract arrays from stream"),
    CMD("stream-foreach",    cmd_stream,     stream_foreach,    true,          BUILTIN,
            "Run a command for each array in a stream"),
    CMD("stream-grep",       cmd_stream,     stream_grep,       true,          BUILTIN,
            "Select arrays from stream based on checks"),
    CMD("stream-merge",      cmd_stream,     stream_merge,      true,          BUILTIN,
            "Merge arrays into stream"),
    CMD("stream-split",      cmd_stream,     stream_split,      true,          BUILTIN,
            "Split stream into separate arrays"),
    CMD("tag",               cmd_array,      tag,               true,          BUILTIN,
            "Set, unset, and query array tags"),
    CMD("to",                cmd_conversion, to,                true,          BUILTIN,
            "Export arrays, autodetect output format"),
    CMD("to-csv",            cmd_conversion, to_csv,            WITH_CSV,      "conv-csv",
            "Export arrays to CSV files"),
    CMD("to-datraw",         cmd_conversion, to_datraw,         WITH_DATRAW,   "conv-datraw",
            "Export arrays to .dat/.raw volume data"),
    CMD("to-exr",            cmd_conversion, to_exr,            WITH_EXR,      "conv-exr",
            "Export arrays to OpenEXR images"),
    CMD("to-gdal",           cmd_conversion, to_gdal,           WITH_GDAL,     "conv-gdal",
            "Export arrays to remote sensing formats via GDAL"),
    CMD("to-jpeg",           cmd_conversion, to_jpeg,           WITH_JPEG,     "conv-jpeg",
            "Export arrays to JPEG images"),
    CMD("to-magick",         cmd_conversion, to_magick,         WITH_MAGICK,   "conv-magick",
            "Export arrays to images via " MAGICK_FLAVOR),
    CMD("to-mat",            cmd_conversion, to_mat,            WITH_MAT,      "conv-mat",
            "Export arrays to Matlab files"),
    CMD("to-netcdf",         cmd_conversion, to_netcdf,         WITH_NETCDF,   "conv-netcdf",
            "Export arrays to NetCDF files"),
    CMD("to-netpbm",         cmd_conversion, to_netpbm,         WITH_NETPBM,   "conv-netpbm",
            "Export arrays to NetPBM images"),
    CMD("to-pcd",            cmd_conversion, to_pcd,            WITH_PCD,      "conv-pcd",
            "Export arrays to PCD point cloud data"),
    CMD("to-pfs",            cmd_conversion, to_pfs,            WITH_PFS,      "conv-pfs",
            "Export arrays to PFS images"),
    CMD("to-ply",            cmd_conversion, to_ply,            WITH_PLY,      "conv-ply",
            "Export arrays to PLY point data"),
    CMD("to-png",            cmd_conversion, to_png,            WITH_PNG,      "conv-png",
            "Export arrays to PNG files"),
    CMD("to-pvm",            cmd_conversion, to_pvm,            WITH_PVM,      "conv-pvm",
            "Export arrays to PVM volume data"),
    CMD("to-rat",            cmd_conversion, to_rat,            WITH_RAT,      "conv-rat",
            "Export arrays to RAT RadarTools data"),
    CMD("to-raw",            cmd_conversion, to_raw,            WITH_RAW,      "conv-raw",
            "Export arrays to raw binary data"),
    CMD("to-sndfile",        cmd_conversion, to_sndfile,        WITH_SNDFILE,  "conv-sndfile",
            "Export arrays to audio files via libsndfile"),
    CMD("to-teem",           cmd_conversion, to_teem,           WITH_TEEM,     "conv-teem",
            "Export arrays to NRRD files via Teem"),
    CMD("uncompress",        cmd_array,      uncompress,        true,          BUILTIN,
            "Uncompress arrays"),
    CMD("version",           cmd_misc,       version,           true,          BUILTIN,
            "Show program version"),
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

const char *cmd_description(int cmd_index)
{
    return cmds[cmd_index].description;
}

cmd_category_t cmd_category(int cmd_index)
{
    return cmds[cmd_index].category;
}

#if DYNAMIC_MODULES
static std::string cmd_module_name(int cmd_index)
{
    const char* env_plugin_path = std::getenv("GTATOOL_PLUGIN_PATH");
    std::string plugin_path = std::string(env_plugin_path ? env_plugin_path : PKGLIBDIR);
    std::string module_name = plugin_path + "/" + cmds[cmd_index].module_name + ".so";
    return module_name;
}
#endif

bool cmd_is_available(int cmd_index)
{
    bool available = cmds[cmd_index].available;
#if DYNAMIC_MODULES
    if (available && !cmds[cmd_index].cmd)
    {
        available = fio::test_f(cmd_module_name(cmd_index));
    }
#endif
    return available;
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
    cmd_t key = { cmd, cmd_misc, false, NULL, NULL, NULL, NULL, NULL };

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
        std::string fn_name = std::string("gtatool_") + str::replace(cmd_name, "-", "_");
        std::string help_fn_name = fn_name + "_help";
        // We used RTLD_LAZY here, but that broke the from-dcmtk command whith dmctk 3.6.0.
        cmds[cmd_index].module_handle = dlopen(cmd_module_name(cmd_index).c_str(), RTLD_NOW);
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

#if DYNAMIC_MODULES
void* cmd_symbol(int cmd_index, const char* symbol)
{
    return dlsym(cmds[cmd_index].module_handle, symbol);
}
#endif

void cmd_close(int cmd_index)
{
#if DYNAMIC_MODULES
    if (cmds[cmd_index].module_handle)
    {
        // XXX HACK for Qt5: do not close a module using Qt5 because that library
        // seems to register atexit() handlers, which results in a jump to an
        // invalid address when main() returns but the handlers are not present anymore.
        if (strcmp(cmds[cmd_index].name, "gui") == 0)
            return;
        (void)dlclose(cmds[cmd_index].module_handle);
        cmds[cmd_index].module_handle = NULL;
        cmds[cmd_index].cmd_print_help = NULL;
        cmds[cmd_index].cmd = NULL;
    }
#endif
}
