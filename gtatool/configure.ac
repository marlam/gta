dnl Process this file with autoconf to produce a configure script.

dnl configure.ac
dnl
dnl This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
dnl (GTAs).
dnl
dnl Copyright (C) 2010, 2011, 2012
dnl Martin Lambers <marlam@marlam.de>
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see <http://www.gnu.org/licenses/>.

dnl Autotools init stuff
AC_PREREQ([2.65])
AC_INIT([gtatool], [1.1.0], [gta-list@nongnu.org], [gtatool], [http://gta.nongnu.org/])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_TARGET
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AM_INIT_AUTOMAKE([1.11.1 color-tests silent-rules tar-pax -Wall -Wno-portability])
AM_SILENT_RULES([yes])
LT_PREREQ([2.2.6])
LT_INIT([disable-static])
AC_PROG_CXX
AC_PROG_INSTALL
AC_LANG([C++])
AC_C_BIGENDIAN

dnl Useful definitions
AC_DEFINE_UNQUOTED([PLATFORM], ["${target}"], [Platform triplet])

dnl Special handling of W32
case "${target}" in *-*-mingw32*) w32="yes" ;; *) w32="no " ;; esac
if test "$w32" = "yes"; then
    AC_CHECK_TOOL([WINDRES], [windres], [:])
    if test "$WINDRES" = ":"; then
        AC_MSG_ERROR([windres not found])
    fi
    AC_CHECK_TOOL([MAKENSIS], [makensis], [:])
    if test "$MAKENSIS" = ":"; then
        AC_MSG_WARN([makensis not found: you will not be able to create a W32 installer])
    fi
    dnl Define _BSD_SOURCE to get the optreset declaration. We need this
    dnl because we may use getopt_long for more than one command line.
    CPPFLAGS="$CPPFLAGS -D_BSD_SOURCE"
    dnl For PathMatchSpec() in base module fio:
    LIBS="$LIBS -lshlwapi"
else
    MAKENSIS=":"
fi
AM_CONDITIONAL([HAVE_MAKENSIS], [test "$MAKENSIS" != ":"])
AM_CONDITIONAL([W32], [test "$w32" = "yes"])
if test "$w32" = "yes"; then W32=1; else W32=0; fi
AC_DEFINE_UNQUOTED([W32], [$W32], [Use W32 API?])

dnl Valgrind
AC_ARG_ENABLE([valgrind-tests],
    [AS_HELP_STRING([--enable-valgrind-tests], [Use valgrind when running tests. Disabled by default.])],
    [if test "$enableval" = "yes"; then valgrind_tests="yes"; else valgrind_tests="no "; fi], [valgrind_tests="no "])
AC_ARG_VAR([VALGRIND], [Valgrind command])
if test "$valgrind_tests" = "yes"; then
    AC_CHECK_PROGS([VALGRIND], [valgrind], [:])
    if test "$VALGRIND" = ":"; then
        valgrind_tests="no "
        AC_MSG_WARN([Disabled valgrind tests because valgrind was not found.])
    fi
fi
AM_CONDITIONAL([VALGRIND_TESTS], [test "$valgrind_tests" = "yes"])

dnl Dynamic modules
LIB_DLOPEN=
AC_SUBST([LIB_DLOPEN])
SAVED_LIBS="$LIBS"
AC_SEARCH_LIBS([dlopen], [dl],
    [test "$ac_cv_search_dlopen" = "none required" || LIB_DLOPEN="$ac_cv_search_dlopen"])
AC_CHECK_FUNCS([dlopen])
LIBS="$SAVED_LIBS"
AC_ARG_ENABLE([dynamic-modules],
    [AS_HELP_STRING([--enable-dynamic-modules], [Enable dynamic loadable modules. Enabled by default if dlopen() is available.])],
    [if test "$enableval" = "yes"; then dynamic_modules="yes"; else dynamic_modules="no "; fi], 
    [if test "$ac_cv_func_dlopen" = "no"; then dynamic_modules="no "; else dynamic_modules="yes"; fi])
AC_DEFINE_UNQUOTED([DYNAMIC_MODULES], [`if test "$dynamic_modules" = "yes"; then echo "1"; else echo "0"; fi`], [Use dynamic modules?])
AM_CONDITIONAL([DYNAMIC_MODULES], [test "$dynamic_modules" = "yes"])

dnl Configure switches
AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([--enable-debug], [Enable debugging checks. Disabled by default.])],
    [if test "$enableval" = "yes"; then ndebug=0; else ndebug=1; fi], [ndebug=1])
if test "$ndebug" = "1"; then
    AC_DEFINE_UNQUOTED([NDEBUG], [1], [Define to 1 to disable assertions and other debugging checks.])
fi

dnl Feature checks needed by the base modules
dnl - dbg
AC_CHECK_FUNCS([backtrace sigaction])
dnl - endianness
AC_C_BIGENDIAN
dnl - fio
case "${target}" in *-*-mingw*) LIBS="$LIBS -lshlwapi" ;; esac
AC_CHECK_FUNCS([fdatasync fnmatch fseeko ftello getpwuid link mmap posix_fadvise readdir_r symlink])
dnl - opt
case "${target}" in *-*-mingw*) CPPFLAGS="$CPPFLAGS -D_BSD_SOURCE" ;; esac
AC_CHECK_DECLS([optreset], [], [], [#include <getopt.h>])
dnl - str
AM_ICONV([])
AC_CHECK_FUNCS([nl_langinfo vasprintf wcswidth])
dnl - sys
dnl AC_CHECK_FUNCS([nanosleep sysconf sched_yield])
dnl - thread
AC_MSG_CHECKING([for GCC atomic builtins])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]],
    [[int b, x = 0; b = __sync_bool_compare_and_swap(&x, 0, 1);]])],
    HAVE_GCC_ATOMIC_BUILTINS=1, HAVE_GCC_ATOMIC_BUILTINS=0)
if test "$HAVE_GCC_ATOMIC_BUILTINS" = "1"; then
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([GCC atomic builtins not found. Please use a newer compiler.])
fi
AC_SEARCH_LIBS([pthread_create], [pthread])
CPPFLAGS_bak="$CPPFLAGS"
LDFLAGS_bak="$LDFLAGS"
CPPFLAGS="-pthread $CPPFLAGS"
LDFLAGS="-pthread $LDFLAGS"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])], [], [CPPFLAGS="$CPPFLAGS_bak"; LDFLAGS="$LDFLAGS_bak"])
dnl - timer
dnl AC_SEARCH_LIBS([clock_gettime], [rt])
dnl AC_CHECK_FUNCS([clock_gettime])

dnl pkg-config
PKG_PROG_PKG_CONFIG([])
if test -z "$PKG_CONFIG"; then
    AC_MSG_ERROR([pkg-config not found: required libraries cannot be detected])
fi

dnl The GTA library
PKG_CHECK_MODULES([libgta], [gta >= 0.9.4])

dnl component-compute: muParser
AC_ARG_WITH([muparser],
    [AS_HELP_STRING([--with-muparser], [Enable the component-compute command. Enabled by default if libmuparser is available.])],
    [if test "$withval" = "yes"; then muparser="yes"; else muparser="no "; fi], [muparser="yes"])
if test "$muparser" = "yes"; then
    PKG_CHECK_MODULES([libmuparser], [muparser >= 0.0], [],
        [muparser="no "
        AC_MSG_WARN([MuParser library not found:])
        AC_MSG_WARN([$libmuparser_PKG_ERRORS])
        AC_MSG_WARN([Disabled the component-compute command.])])
fi
AC_DEFINE_UNQUOTED([WITH_MUPARSER], [`if test "$muparser" = "yes"; then echo "1"; else echo "0"; fi`], [Use muparser?])
AM_CONDITIONAL([WITH_MUPARSER], [test "$muparser" = "yes"])

dnl gui: Qt
AC_ARG_WITH([qt],
    [AS_HELP_STRING([--with-qt], [Enable Qt GUI. Enabled by default if Qt is available.])],
    [if test "$withval" = "yes"; then qt="yes"; else qt="no "; fi], [qt="yes"])
if test "$qt" = "yes"; then
    PKG_CHECK_MODULES([libqtgui], [QtGui >= 4.6], [],
        [qt="no "
        AC_MSG_WARN([QtGui library not found:])
        AC_MSG_WARN([$libqtgui_PKG_ERRORS])
        AC_MSG_WARN([Disabled the gui command.])])
    AC_ARG_VAR([RCC], [rcc command])
    if test -z "$RCC"; then
        AC_CHECK_TOOLS([RCC], [rcc-qt4 rcc])
    fi
    AC_ARG_VAR([MOC], [moc command])
    if test -z "$MOC"; then
        AC_CHECK_TOOLS([MOC], [moc-qt4 moc])
    fi
    dnl Icon and Menu tools. It is ok if these are missing.
    GTK_UPDATE_ICON_CACHE=""
    AC_ARG_VAR([GTK_UPDATE_ICON_CACHE], [gtk-update-icon-cache command])
    AC_CHECK_PROGS([GTK_UPDATE_ICON_CACHE], [gtk-update-icon-cache])
    UPDATE_DESKTOP_DATABASE=""
    AC_ARG_VAR([UPDATE_DESKTOP_DATABASE], [update-desktop-database command])
    AC_CHECK_PROGS([UPDATE_DESKTOP_DATABASE], [update-desktop-database])
fi
AC_DEFINE_UNQUOTED([WITH_QT], [`if test "$qt" = "yes"; then echo "1"; else echo "0"; fi`], [Use Qt?])
AM_CONDITIONAL([WITH_QT], [test "$qt" = "yes"])

dnl conv-csv
AC_ARG_WITH([csv],
    [AS_HELP_STRING([--with-csv], [Enable CSV import/export. Enabled by default.])],
    [if test "$withval" = "yes"; then csv="yes"; else csv="no "; fi], [csv="yes"])
AC_DEFINE_UNQUOTED([WITH_CSV], [`if test "$csv" = "yes"; then echo "1"; else echo "0"; fi`], [Use csv?])
AM_CONDITIONAL([WITH_CSV], [test "$csv" = "yes"])

dnl conv-dcmtk: DCMTK
AC_ARG_WITH([dcmtk],
    [AS_HELP_STRING([--with-dcmtk], [Enable DCMTK import. Enabled by default if libdcmimgle is available.])],
    [if test "$withval" = "yes"; then dcmtk="yes"; else dcmtk="no "; fi], [dcmtk="yes"])
if test "$dcmtk" = "yes"; then
    AC_LIB_FROMPACKAGE([dcmimgle], [dcmtk])
    case "${target}" in
    *-*-mingw32*)
        AC_LIB_HAVE_LINKFLAGS([dcmimgle], [dcmdata dcmjpeg ijg8 ijg12 ijg16 oflog ofstd z ws2_32],
        [#define HAVE_CONFIG_H
         #include <dcmtk/dcmimgle/dcmimage.h>],
        [DicomImage img(NULL);])
        ;;
    *)
        AC_LIB_HAVE_LINKFLAGS([dcmimgle], [dcmdata dcmjpeg ijg8 ijg12 ijg16 oflog ofstd pthread z],
        [#define HAVE_CONFIG_H
         #include <dcmtk/dcmimgle/dcmimage.h>],
        [DicomImage img(NULL);])
        ;;
    esac
    if test "$HAVE_LIBDCMIMGLE" != "yes"; then
        dcmtk="no "
        AC_MSG_WARN([DCMTK library not found.])
        AC_MSG_WARN([Disabled the from-dcmtk command.])
    fi
fi
AC_DEFINE_UNQUOTED([WITH_DCMTK], [`if test "$dcmtk" = "yes"; then echo "1"; else echo "0"; fi`], [Use dcmtk?])
AM_CONDITIONAL([WITH_DCMTK], [test "$dcmtk" = "yes"])

dnl conv-exr: OpenEXR
AC_ARG_WITH([exr],
    [AS_HELP_STRING([--with-exr], [Enable OpenEXR import/export. Enabled by default if libIlmImf is available.])],
    [if test "$withval" = "yes"; then exr="yes"; else exr="no "; fi], [exr="yes"])
if test "$exr" = "yes"; then
    PKG_CHECK_MODULES([libopenexr], [OpenEXR >= 0.0], [],
        [exr="no "
        AC_MSG_WARN([OpenEXR library not found:])
        AC_MSG_WARN([$libopenexr_PKG_ERRORS])
        AC_MSG_WARN([Disabled the from-exr and to-exr commands.])])
fi
AC_DEFINE_UNQUOTED([WITH_EXR], [`if test "$exr" = "yes"; then echo "1"; else echo "0"; fi`], [Use exr?])
AM_CONDITIONAL([WITH_EXR], [test "$exr" = "yes"])

dnl conv-ffmpeg: FFmpeg
AC_ARG_WITH([ffmpeg],
    [AS_HELP_STRING([--with-ffmpeg], [Enable FFmpeg import/export. Enabled by default if libavcodec is available.])],
    [if test "$withval" = "yes"; then ffmpeg="yes"; else ffmpeg="no "; fi], [ffmpeg="yes"])
if test "$ffmpeg" = "yes"; then
    AC_CHECK_FUNCS([sysconf])
    PKG_CHECK_MODULES([libffmpeg], [libavformat >= 52.110.0 libavdevice libswscale], [],
        [ffmpeg="no "
        AC_MSG_WARN([FFmpeg library not found:])
        AC_MSG_WARN([$libffmpeg_PKG_ERRORS])
        AC_MSG_WARN([Disabled the from-ffmpeg command.])])
fi
AC_DEFINE_UNQUOTED([WITH_FFMPEG], [`if test "$ffmpeg" = "yes"; then echo "1"; else echo "0"; fi`], [Use FFmpeg?])
AM_CONDITIONAL([WITH_FFMPEG], [test "$ffmpeg" = "yes"])

dnl conv-gdal: GDAL
AC_ARG_WITH([gdal],
    [AS_HELP_STRING([--with-gdal], [Enable GDAL import/export. Enabled by default if libgdal is available.])],
    [if test "$withval" = "yes"; then gdal="yes"; else gdal="no "; fi], [gdal="yes"])
if test "$gdal" = "yes"; then
    AC_ARG_VAR([GDALCONFIG], [gdal-config command])
    if test -z "$GDALCONFIG"; then
        AC_CHECK_TOOLS([GDALCONFIG], [gdal-config], [])
    fi
    if test -n "$GDALCONFIG"; then
        libgdal_CFLAGS="`$GDALCONFIG --cflags`"
        libgdal_LIBS="`$GDALCONFIG --libs`"
        AC_SUBST([libgdal_CFLAGS])
        AC_SUBST([libgdal_LIBS])
    else
        gdal="no "
        AC_MSG_WARN([GDAL library not found.])
        AC_MSG_WARN([Disabled the from-gdal and to-gdal commands.])
    fi
fi
AC_DEFINE_UNQUOTED([WITH_GDAL], [`if test "$gdal" = "yes"; then echo "1"; else echo "0"; fi`], [Use gdal?])
AM_CONDITIONAL([WITH_GDAL], [test "$gdal" = "yes"])

dnl conv-magick: ImageMagick
AC_ARG_WITH([magick],
    [AS_HELP_STRING([--with-magick], [Enable ImageMagick import/export. Enabled by default if libMagick++ is available.])],
    [if test "$withval" = "yes"; then magick="yes"; else magick="no "; fi], [magick="yes"])
if test "$magick" = "yes"; then
    PKG_CHECK_MODULES([libmagick], [ImageMagick++ >= 0.0], [],
        [magick="no "
        AC_MSG_WARN([ImageMagick++ library not found:])
        AC_MSG_WARN([$libmagick_PKG_ERRORS])
        AC_MSG_WARN([Disabled the from-magick and to-magick commands.])])
fi
AC_DEFINE_UNQUOTED([WITH_MAGICK], [`if test "$magick" = "yes"; then echo "1"; else echo "0"; fi`], [Use magick?])
AM_CONDITIONAL([WITH_MAGICK], [test "$magick" = "yes"])

dnl conv-jpeg: libjpeg
AC_ARG_WITH([jpeg],
    [AS_HELP_STRING([--with-jpeg], [Enable JPEG import/export. Enabled by default if libjpeg is available.])],
    [if test "$withval" = "yes"; then jpeg="yes"; else jpeg="no "; fi], [jpeg="yes"])
if test "$jpeg" = "yes"; then
    AC_LANG([C])
    AC_LIB_FROMPACKAGE([jpeg], [jpeg])
    AC_LIB_HAVE_LINKFLAGS([jpeg], [],
        [#include <stdio.h>
	 #include <jpeglib.h>],
	[jpeg_create_compress(0);])
    AC_LANG([C++])
    if test "$HAVE_LIBJPEG" != "yes"; then
        jpeg="no "
        AC_MSG_WARN([libjpeg library not found.])
        AC_MSG_WARN([Disabled the from-jpeg and to-jpeg commands.])
    fi
fi
AC_DEFINE_UNQUOTED([WITH_JPEG], [`if test "$jpeg" = "yes"; then echo "1"; else echo "0"; fi`], [Use jpeg?])
AM_CONDITIONAL([WITH_JPEG], [test "$jpeg" = "yes"])

dnl conv-mat: MATIO
AC_ARG_WITH([matio],
    [AS_HELP_STRING([--with-matio], [Enable MATLAB import/export. Enabled by default if libmatio is available.])],
    [if test "$withval" = "yes"; then mat="yes"; else mat="no "; fi], [mat="yes"])
if test "$mat" = "yes"; then
    PKG_CHECK_MODULES([libmatio], [matio >= 0.0], [],
        [mat="no "
        AC_MSG_WARN([MATIO library not found:])
        AC_MSG_WARN([$libmatio_PKG_ERRORS])
        AC_MSG_WARN([Disabled the from-mat and to-mat commands.])])
fi
AC_DEFINE_UNQUOTED([WITH_MAT], [`if test "$mat" = "yes"; then echo "1"; else echo "0"; fi`], [Use mat?])
AM_CONDITIONAL([WITH_MAT], [test "$mat" = "yes"])

dnl conv-netpbm: NetPBM
AC_ARG_WITH([netpbm],
    [AS_HELP_STRING([--with-netpbm], [Enable NetPBM import/export. Enabled by default if libnetpbm is available.])],
    [if test "$withval" = "yes"; then netpbm="yes"; else netpbm="no "; fi], [netpbm="yes"])
if test "$netpbm" = "yes"; then
    AC_LANG([C])
    AC_LIB_FROMPACKAGE([netpbm], [netpbm])
    AC_LIB_HAVE_LINKFLAGS([netpbm], [], [#include <pam.h>], [pm_init(0, 0);])
    AC_LANG([C++])
    if test "$HAVE_LIBNETPBM" != "yes"; then
        netpbm="no "
        AC_MSG_WARN([NetPBM library not found.])
        AC_MSG_WARN([Disabled the from-netpbm and to-netpbm commands.])
    fi
fi
AC_DEFINE_UNQUOTED([WITH_NETPBM], [`if test "$netpbm" = "yes"; then echo "1"; else echo "0"; fi`], [Use netpbm?])
AM_CONDITIONAL([WITH_NETPBM], [test "$netpbm" = "yes"])

dnl conv-pcd: libpcl_io
AC_ARG_WITH([pcd],
    [AS_HELP_STRING([--with-pcd], [Enable PCD import/export. Enabled by default if libpcl_io is available.])],
    [if test "$withval" = "yes"; then pcd="yes"; else pcd="no "; fi], [pcd="yes"])
if test "$pcd" = "yes"; then
    PKG_CHECK_MODULES([libpcl_io], [pcl_io >= 1.0], [],
        [pcd="no "
        AC_MSG_WARN([PCL io library not found:])
        AC_MSG_WARN([$libpcl_io_PKG_ERRORS])
        AC_MSG_WARN([Disabled the from-pcd and to-pcd commands.])])
fi
AC_DEFINE_UNQUOTED([WITH_PCD], [`if test "$pcd" = "yes"; then echo "1"; else echo "0"; fi`], [Use pcd?])
AM_CONDITIONAL([WITH_PCD], [test "$pcd" = "yes"])

dnl conv-pfs: PFS
AC_ARG_WITH([pfs],
    [AS_HELP_STRING([--with-pfs], [Enable PFS import/export. Enabled by default if libpfs-1.2 is available.])],
    [if test "$withval" = "yes"; then pfs="yes"; else pfs="no "; fi], [pfs="yes"])
if test "$pfs" = "yes"; then
    PKG_CHECK_MODULES([libpfs], [pfs >= 0.0], [],
        [pfs="no "
        AC_MSG_WARN([PFS library not found:])
        AC_MSG_WARN([$libpfs_PKG_ERRORS])
        AC_MSG_WARN([Disabled the from-pfs and to-pfs commands.])])
fi
AC_DEFINE_UNQUOTED([WITH_PFS], [`if test "$pfs" = "yes"; then echo "1"; else echo "0"; fi`], [Use pfs?])
AM_CONDITIONAL([WITH_PFS], [test "$pfs" = "yes"])

dnl conv-ply
AC_ARG_WITH([ply],
    [AS_HELP_STRING([--with-ply], [Enable PLY import/export. Enabled by default.])],
    [if test "$withval" = "yes"; then ply="yes"; else ply="no "; fi], [ply="yes"])
AC_DEFINE_UNQUOTED([WITH_PLY], [`if test "$ply" = "yes"; then echo "1"; else echo "0"; fi`], [Use ply?])
AM_CONDITIONAL([WITH_PLY], [test "$ply" = "yes"])

dnl conv-pvm
AC_ARG_WITH([pvm],
    [AS_HELP_STRING([--with-pvm], [Enable PVM import/export. Enabled by default.])],
    [if test "$withval" = "yes"; then pvm="yes"; else pvm="no "; fi], [pvm="yes"])
AC_DEFINE_UNQUOTED([WITH_PVM], [`if test "$pvm" = "yes"; then echo "1"; else echo "0"; fi`], [Use pvm?])
AM_CONDITIONAL([WITH_PVM], [test "$pvm" = "yes"])

dnl conv-rat
AC_ARG_WITH([rat],
    [AS_HELP_STRING([--with-rat], [Enable RAT import/export. Enabled by default.])],
    [if test "$withval" = "yes"; then rat="yes"; else rat="no "; fi], [rat="yes"])
AC_DEFINE_UNQUOTED([WITH_RAT], [`if test "$rat" = "yes"; then echo "1"; else echo "0"; fi`], [Use rat?])
AM_CONDITIONAL([WITH_RAT], [test "$rat" = "yes"])

dnl conv-raw
AC_ARG_WITH([raw],
    [AS_HELP_STRING([--with-raw], [Enable RAW import/export. Enabled by default.])],
    [if test "$withval" = "yes"; then raw="yes"; else raw="no "; fi], [raw="yes"])
AC_DEFINE_UNQUOTED([WITH_RAW], [`if test "$raw" = "yes"; then echo "1"; else echo "0"; fi`], [Use raw?])
AM_CONDITIONAL([WITH_RAW], [test "$raw" = "yes"])

dnl conv-sndfile: libsndfile
AC_ARG_WITH([sndfile],
    [AS_HELP_STRING([--with-sndfile], [Enable SNDFILE import/export. Enabled by default if libsndfile is available.])],
    [if test "$withval" = "yes"; then sndfile="yes"; else sndfile="no "; fi], [sndfile="yes"])
if test "$sndfile" = "yes"; then
    PKG_CHECK_MODULES([libsndfile], [sndfile >= 0.0], [],
        [sndfile="no "
        AC_MSG_WARN([sndfile library not found:])
        AC_MSG_WARN([$libsndfile_PKG_ERRORS])
        AC_MSG_WARN([Disabled the from-sndfile and to-sndfile commands.])])
fi
AC_DEFINE_UNQUOTED([WITH_SNDFILE], [`if test "$sndfile" = "yes"; then echo "1"; else echo "0"; fi`], [Use sndfile?])
AM_CONDITIONAL([WITH_SNDFILE], [test "$sndfile" = "yes"])

dnl conv-teem: libteem
AC_ARG_WITH([teem],
    [AS_HELP_STRING([--with-teem], [Enable teem (nrrd) import/export. Enabled by default if libteem is available.])],
    [if test "$withval" = "yes"; then teem="yes"; else teem="no "; fi], [teem="yes"])
if test "$teem" = "yes"; then
    AC_LANG([C])
    AC_LIB_FROMPACKAGE([teem], [teem])
    AC_LIB_HAVE_LINKFLAGS([teem], [], [#include <teem/nrrd.h>], [nrrdNuke(0);])
    AC_LANG([C++])
    if test "$HAVE_LIBTEEM" != "yes"; then
        teem="no "
        AC_MSG_WARN([teem library not found.])
        AC_MSG_WARN([Disabled the from-teem and to-teem commands.])
    fi
fi
AC_DEFINE_UNQUOTED([WITH_TEEM], [`if test "$teem" = "yes"; then echo "1"; else echo "0"; fi`], [Use teem?])
AM_CONDITIONAL([WITH_TEEM], [test "$teem" = "yes"])

dnl Output
AC_CONFIG_FILES([Makefile \
    src/Makefile \
    src/base/Makefile \
    doc/Makefile \
    tests/Makefile \
])
AC_OUTPUT

echo ""
echo "Dynamic modules:         " "$dynamic_modules" "(Requires system support)"
echo ""
echo "component-compute:       " "$muparser" "(Requires muParser, Debian package: libmuparser-dev)"
echo "from-csv, to-csv:        " "$csv" ""
echo "from-dcmtk:              " "$dcmtk" "(Requires DCMTK, Debian package: libdcmtk2-dev)"
echo "from-exr, to-exr:        " "$exr" "(Requires OpenEXR, Debian package: libopenexr-dev)"
echo "from-ffmpeg:             " "$ffmpeg" "(Requires FFmpeg, Debian package: libavcodec-dev)"
echo "from-gdal, to-gdal:      " "$gdal" "(Requires GDAL, Debian package: libgdal-dev)"
echo "from-jpeg, to-jpeg:      " "$jpeg" "(Requires libjpeg, Debian package: libjpeg-dev)"
echo "from-magick, to-magick:  " "$magick" "(Requires ImageMagick, Debian package: libmagick++-dev)"
echo "from-mat, to-mat:        " "$mat" "(Requires matio, Debian package: libmatio-dev)"
echo "from-netpbm, to-netpbm:  " "$netpbm" "(Requires NetPBM, Debian package: libnetpbm10-dev)"
echo "from-pcd, to-pcd:        " "$pcd" "(Requires the Point Cloud Library IO module libpcl_io)"
echo "from-pfs, to-pfs:        " "$pfs" "(Requires pfstools, Debian package: libpfs-dev)"
echo "from-ply, to-ply:        " "$ply" ""
echo "from-pvm, to-pvm:        " "$pvm" ""
echo "from-rat, to-rat:        " "$rat" ""
echo "from-raw, to-raw:        " "$raw" ""
echo "from-sndfile, to-sndfile:" "$sndfile" "(Requires libsndfile, Debian package: libsndfile1-dev)"
echo "from-teem, to-teem:      " "$teem" "(Requires libteem, Debian package: libteem1-dev)"
echo "gui:                     " "$qt" "(Requires Qt, Debian package: libqt4-dev)"
echo ""
