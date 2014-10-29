dnl Process this file with autoconf to produce a configure script.

dnl configure.ac
dnl
dnl This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
dnl (GTAs).
dnl
dnl Copyright (C) 2010, 2011, 2012, 2013, 2014
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
AC_INIT([GTA Tool], [2.0.1], [gta-list@nongnu.org], [gtatool], [http://gta.nongnu.org/])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_TARGET
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AM_INIT_AUTOMAKE([1.11.1 color-tests parallel-tests silent-rules subdir-objects tar-pax -Wall -Wno-portability])
AM_SILENT_RULES([yes])
LT_PREREQ([2.2.6])
LT_INIT([disable-static])
AC_PROG_CXX
AC_PROG_INSTALL
AC_LANG([C++])
AC_C_BIGENDIAN

dnl Check if the C++ compiler works. Autoconf will not do that for us!?
dnl We want C++11, and blindly add -std=gnu++11. This may only work for gcc and clang...
CXXFLAGS="-std=gnu++11 $CXXFLAGS"
AC_MSG_CHECKING([whether the C++ compiler works])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])], [COMPILER_WORKS=1], [COMPILER_WORKS=0])
if test "$COMPILER_WORKS" = "1"; then
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([The C++ compiler was not found or does not work.])
fi

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
DLOPEN_LIBS=
AC_SUBST([DLOPEN_LIBS])
SAVED_LIBS="$LIBS"
AC_SEARCH_LIBS([dlopen], [dl],
    [test "$ac_cv_search_dlopen" = "none required" || DLOPEN_LIBS="$ac_cv_search_dlopen"])
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
BASE_LIBS=
AC_SUBST([BASE_LIBS])
SAVED_LIBS="$LIBS"
AC_LANG_PUSH([C])
dnl - dbg
AC_CHECK_FUNCS([backtrace sigaction])
dnl - end
AC_C_BIGENDIAN
dnl - fio
case "${target}" in *-*-mingw*) LIBS="$LIBS -lshlwapi" ;; esac
AC_CHECK_FUNCS([fdatasync fnmatch fseeko ftello getpwuid link mmap posix_fadvise readdir_r symlink])
dnl - opt
case "${target}" in *-*-mingw*) CPPFLAGS="$CPPFLAGS -D_BSD_SOURCE" ;; esac
AC_CHECK_DECLS([optreset], [], [], [#include <getopt.h>])
dnl - pth
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
AC_SEARCH_LIBS([pthread_create], [pthread],
    [test "$ac_cv_search_pthread_create" = "none required" || BASE_LIBS="$BASE_LIBS $ac_cv_search_pthread_create"])
CPPFLAGS_bak="$CPPFLAGS"
LDFLAGS_bak="$LDFLAGS"
CPPFLAGS="-pthread $CPPFLAGS"
LDFLAGS="-pthread $LDFLAGS"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])], [], [CPPFLAGS="$CPPFLAGS_bak"; LDFLAGS="$LDFLAGS_bak"])
dnl - str
AM_ICONV([])
AC_CHECK_FUNCS([nl_langinfo vasprintf wcswidth])
AC_CHECK_TYPES([int128_t, uint128_t], [], [], [[#include <cstdint>]])
AC_CHECK_TYPES([__int128, unsigned __int128])
AC_CHECK_TYPES([__float128])
AC_SEARCH_LIBS([strtoflt128], [quadmath],
    [test "$ac_cv_search_strtoflt128" = "none required" || BASE_LIBS="$BASE_LIBS $ac_cv_search_strtoflt128"])
dnl TODO: define LONG_DOUBLE_IS_IEEE_754_QUAD on systems where this is true
dnl - sys
AC_CHECK_FUNCS([nanosleep sysconf sched_yield])
dnl - tmr
AC_SEARCH_LIBS([clock_gettime], [rt],
    [test "$ac_cv_search_clock_gettime" = "none required" || BASE_LIBS="$BASE_LIBS $ac_cv_search_clock_gettime"])
AC_CHECK_FUNCS([clock_gettime])
AC_LANG_POP([C])
LIBS="$SAVED_LIBS"

dnl pkg-config
PKG_PROG_PKG_CONFIG([])
if test -z "$PKG_CONFIG"; then
    AC_MSG_ERROR([pkg-config not found: required libraries cannot be detected])
fi

dnl The GTA library
PKG_CHECK_MODULES([libgta], [gta >= 0.9.4])

dnl Choose Qt version for the gui and view command. Qt4 remains the default until Qt5 is rock solid.
qt_version=""
AC_ARG_WITH([qt-version],
    [AS_HELP_STRING([--with-qt-version], [Choose Qt version. Currently 4 or 5; 4 is the default.])],
    [qt_version="$withval"], [qt_version="4"])
if test "$qt_version" != "4" -a "$qt_version" != "5"; then
    AC_MSG_WARN([unknown Qt version requested, falling back to Qt 4.])
    qt_version="4"
fi

dnl Choose Magick flavor for the from-magick and to-magick commands: GraphicsMagick (default) or ImageMagick.
magick_flavor=""
AC_ARG_WITH([magick-flavor],
    [AS_HELP_STRING([--with-magick-flavor], [Choose Magick flavor: GraphicsMagick (default) or ImageMagick.])],
    [magick_flavor="$withval"], [magick_flavor="GraphicsMagick"])
if test "$magick_flavor" != "GraphicsMagick" -a "$magick_flavor" != "ImageMagick"; then
    AC_MSG_WARN([unknown Magick flavor requested, falling back to GraphicsMagick.])
    magick_flavor="GraphicsMagick"
fi
AC_DEFINE_UNQUOTED([MAGICK_FLAVOR], ["${magick_flavor}"], [Magick flavor to use])

dnl stream-foreach: some checks required
AC_CHECK_FUNCS([sigaction])
AC_CHECK_HEADERS([sys/wait.h])

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

dnl gui: Qt.
AC_ARG_WITH([qt],
    [AS_HELP_STRING([--with-qt], [Enable Qt GUI. Enabled by default if Qt is available.])],
    [if test "$withval" = "yes"; then qt="yes"; else qt="no "; fi], [qt="yes"])
if test "$qt" = "yes"; then
    AC_ARG_VAR([RCC], [rcc command])
    AC_ARG_VAR([MOC], [moc command])
    if test "$qt_version" = "4"; then
        PKG_CHECK_MODULES([libqtgui], [QtGui >= 4.8], [],
            [qt="no "
            AC_MSG_WARN([QtGui library (Qt4) not found:])
            AC_MSG_WARN([$libqtgui_PKG_ERRORS])
            AC_MSG_WARN([Disabled the gui and view commands.])
	    AC_MSG_WARN([(Note: you can select Qt5 with --with-qt-version=5)])])
        if test "$qt" = "yes"; then
            if test -z "$RCC"; then
                AC_CHECK_TOOLS([RCC], [rcc-qt4 rcc])
            fi
            if test -z "$MOC"; then
                AC_CHECK_TOOLS([MOC], [moc-qt4 moc])
            fi
        fi
    else
        PKG_CHECK_MODULES([libqtgui], [Qt5Widgets >= 5.0], [],
            [qt="no "
            AC_MSG_WARN([Qt5Widgets library (Qt5) not found:])
            AC_MSG_WARN([$libqtgui_PKG_ERRORS])
            AC_MSG_WARN([Disabled the gui and view commands.])
	    AC_MSG_WARN([(Note: you can select Qt4 with --with-qt-version=4)])])
        if test "$qt" = "yes"; then
            if test -z "$RCC"; then
                AC_CHECK_TOOLS([RCC], [rcc-qt5 rcc])
            fi
            if test -z "$MOC"; then
                AC_CHECK_TOOLS([MOC], [moc-qt5 moc])
            fi
            dnl For a static Windows build with MXE, we need to statically link the
            dnl QWindowsIntegrationPlugin and AccessibleFactory plugins.
            if test "$w32" = "yes"; then
                libqtgui_plugin_L="`$PKG_CONFIG --libs-only-L Qt5Core | $SED -e 's/ //g'`/../plugins"
                libqtgui_LIBS="$libqtgui_plugin_L/platforms -lqwindows $libqtgui_plugin_L/accessible -lqtaccessiblewidgets $libqtgui_LIBS"
                libqtgui_CFLAGS="$libqtgui_CFLAGS -DQT_STATICPLUGIN"
            fi
        fi
    fi
fi
if test "$qt" = "yes"; then
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

dnl conv-datraw
AC_ARG_WITH([datraw],
    [AS_HELP_STRING([--with-datraw], [Enable import/export of the .dat/.raw volumetric data format. Enabled by default.])],
    [if test "$withval" = "yes"; then datraw="yes"; else datraw="no "; fi], [datraw="yes"])
AC_DEFINE_UNQUOTED([WITH_DATRAW], [`if test "$datraw" = "yes"; then echo "1"; else echo "0"; fi`], [Use datraw?])
AM_CONDITIONAL([WITH_DATRAW], [test "$datraw" = "yes"])

dnl conv-dcmtk: DCMTK
AC_ARG_WITH([dcmtk],
    [AS_HELP_STRING([--with-dcmtk], [Enable DCMTK import. Enabled by default if libdcmimgle is available.])],
    [if test "$withval" = "yes"; then dcmtk="yes"; else dcmtk="no "; fi], [dcmtk="yes"])
if test "$dcmtk" = "yes"; then
    AC_LIB_FROMPACKAGE([dcmimgle], [dcmtk])
    case "${target}" in
    *-*-mingw32*)
        AC_LIB_HAVE_LINKFLAGS([dcmimage], [dcmimgle dcmdata dcmjpeg ijg8 ijg12 ijg16 oflog ofstd z ws2_32],
        [#define HAVE_CONFIG_H
         #include <dcmtk/dcmimgle/dcmimage.h>],
        [DicomImage img(NULL);])
        ;;
    *)
        AC_LIB_HAVE_LINKFLAGS([dcmimage], [dcmimgle dcmdata dcmjpeg ijg8 ijg12 ijg16 oflog ofstd pthread z],
        [#define HAVE_CONFIG_H
         #include <dcmtk/dcmimgle/dcmimage.h>],
        [DicomImage img(NULL);])
        ;;
    esac
    if test "$HAVE_LIBDCMIMAGE" != "yes"; then
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

dnl conv-magick: *Magick
AC_ARG_WITH([magick],
    [AS_HELP_STRING([--with-magick], [Enable *Magick import/export. Enabled by default if the flavor chosen with --with-magick-flavor is available.])],
    [if test "$withval" = "yes"; then magick="yes"; else magick="no "; fi], [magick="yes"])
if test "$magick" = "yes"; then
    if test "$magick_flavor" = "GraphicsMagick"; then
        PKG_CHECK_MODULES([libmagick], [GraphicsMagick++ >= 0.0], [],
            [magick="no "
            AC_MSG_WARN([GraphicsMagick++ library not found:])
            AC_MSG_WARN([$libmagick_PKG_ERRORS])
            AC_MSG_WARN([Disabled the from-magick and to-magick commands.])])
    else
        PKG_CHECK_MODULES([libmagick], [ImageMagick++ >= 0.0], [],
            [magick="no "
            AC_MSG_WARN([ImageMagick++ library not found:])
            AC_MSG_WARN([$libmagick_PKG_ERRORS])
            AC_MSG_WARN([Disabled the from-magick and to-magick commands.])])
    fi
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
    PKG_CHECK_MODULES([libmatio], [matio >= 1.5.0], [],
        [mat="no "
        AC_MSG_WARN([MATIO library not found:])
        AC_MSG_WARN([$libmatio_PKG_ERRORS])
        AC_MSG_WARN([Disabled the from-mat and to-mat commands.])])
fi
AC_DEFINE_UNQUOTED([WITH_MAT], [`if test "$mat" = "yes"; then echo "1"; else echo "0"; fi`], [Use mat?])
AM_CONDITIONAL([WITH_MAT], [test "$mat" = "yes"])

dnl conv-netcdf: NetCDF
AC_ARG_WITH([netcdf],
    [AS_HELP_STRING([--with-netcdf], [Enable NetCDF import/export. Enabled by default if libnetcdf is available.])],
    [if test "$withval" = "yes"; then netcdf="yes"; else netcdf="no "; fi], [netcdf="yes"])
if test "$netcdf" = "yes"; then
    PKG_CHECK_MODULES([libnetcdf], [netcdf >= 0.0], [],
        [netcdf="no "
        AC_MSG_WARN([NetCDF library not found:])
        AC_MSG_WARN([$libnetcdf_PKG_ERRORS])
        AC_MSG_WARN([Disabled the from-netcdf and to-netcdf commands.])])
fi
AC_DEFINE_UNQUOTED([WITH_NETCDF], [`if test "$netcdf" = "yes"; then echo "1"; else echo "0"; fi`], [Use netcdf?])
AM_CONDITIONAL([WITH_NETCDF], [test "$netcdf" = "yes"])

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
    dnl The PCL team puts the PCL version number into the module name. Ouch.
    PKG_CHECK_MODULES([libpcl_io], [pcl_io >= 1.6], [],
        [PKG_CHECK_MODULES([libpcl_io], [pcl_io-1.7 >= 1.7], [],
            [PKG_CHECK_MODULES([libpcl_io], [pcl_io-1.6 >= 1.6], [],
                [pcd="no "
                AC_MSG_WARN([PCL io library not found:])
                AC_MSG_WARN([$libpcl_io_PKG_ERRORS])
                AC_MSG_WARN([Disabled the from-pcd and to-pcd commands.])])])])
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

dnl conv-pmd
AC_ARG_WITH([pmd],
    [AS_HELP_STRING([--with-pmd], [Enable PMD import. Disabled by default. Requires the proprietary libpmdaccess2.])],
    [if test "$withval" = "yes"; then pmd="yes"; else pmd="no "; fi], [pmd="no "])
if test "$pmd" = "yes"; then
    AC_LIB_FROMPACKAGE([pmdaccess2], [pmdaccess2])
    AC_LIB_HAVE_LINKFLAGS([pmdaccess2], [], [#include <pmdsdk2.h>], [pmdOpen(0, 0, 0, 0, 0);])
    if test "$HAVE_LIBPMDACCESS2" != "yes"; then
        pmd="no "
        AC_MSG_WARN([pmdaccess2 library not found.])
        AC_MSG_WARN([Disabled the from-pmd command.])
    fi
fi
AC_DEFINE_UNQUOTED([WITH_PMD], [`if test "$pmd" = "yes"; then echo "1"; else echo "0"; fi`], [Use pmd?])
AM_CONDITIONAL([WITH_PMD], [test "$pmd" = "yes"])

dnl conv-png: libpng
AC_ARG_WITH([png],
    [AS_HELP_STRING([--with-png], [Enable PNG import/export. Enabled by default if libpng is available.])],
    [if test "$withval" = "yes"; then png="yes"; else png="no "; fi], [png="yes"])
if test "$png" = "yes"; then
    PKG_CHECK_MODULES([libpng], [libpng >= 1.2.0], [],
        [png="no "
        AC_MSG_WARN([PNG library not found:])
        AC_MSG_WARN([$libpng_PKG_ERRORS])
        AC_MSG_WARN([Disabled the from-png and to-png commands.])])
fi
AC_DEFINE_UNQUOTED([WITH_PNG], [`if test "$png" = "yes"; then echo "1"; else echo "0"; fi`], [Use png?])
AM_CONDITIONAL([WITH_PNG], [test "$png" = "yes"])

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
    case "${target}" in
    *-*-mingw32*)
        AC_LIB_HAVE_LINKFLAGS([teem], [png bz2 z], [#include <teem/nrrd.h>], [nrrdNuke(0);])
        ;;
    *)
        AC_LIB_HAVE_LINKFLAGS([teem], [], [#include <teem/nrrd.h>], [nrrdNuke(0);])
        ;;
    esac
    AC_LANG([C++])
    if test "$HAVE_LIBTEEM" != "yes"; then
        teem="no "
        AC_MSG_WARN([teem library not found.])
        AC_MSG_WARN([Disabled the from-teem and to-teem commands.])
    fi
fi
AC_DEFINE_UNQUOTED([WITH_TEEM], [`if test "$teem" = "yes"; then echo "1"; else echo "0"; fi`], [Use teem?])
AM_CONDITIONAL([WITH_TEEM], [test "$teem" = "yes"])

dnl view: QtOpenGL and glewmx, and optionally Equalizer and libgls
AC_ARG_WITH([qt-opengl],
    [AS_HELP_STRING([--with-qt-opengl], [Enable visualization via OpenGL. Enabled by default if QtOpenGL is available.])],
    [if test "$withval" = "yes"; then qtopengl="yes"; else qtopengl="no "; fi], [qtopengl="yes"])
if test "$qtopengl" = "yes"; then
    if test "$qt" = "yes"; then
        if test "$qt_version" = "4"; then
            PKG_CHECK_MODULES([libqtopengl], [QtOpenGL >= 4.8], [],
                [qtopengl="no "
                AC_MSG_WARN([QtOpenGL library (Qt4) not found:])
                AC_MSG_WARN([$libqtopengl_PKG_ERRORS])
                AC_MSG_WARN([Disabled the view command.])])
        else
            PKG_CHECK_MODULES([libqtopengl], [Qt5OpenGL >= 5.0], [],
                [qtopengl="no "
                AC_MSG_WARN([Qt5OpenGL library not found:])
                AC_MSG_WARN([$libqtopengl_PKG_ERRORS])
                AC_MSG_WARN([Disabled the view command.])])
        fi
    else
        qtopengl="no "
    fi
fi
AC_DEFINE_UNQUOTED([WITH_QTOPENGL], [`if test "$qtopengl" = "yes"; then echo "1"; else echo "0"; fi`], [Use QtOpenGL?])
AM_CONDITIONAL([WITH_QTOPENGL], [test "$qtopengl" = "yes"])
AC_ARG_WITH([glewmx],
    [AS_HELP_STRING([--with-glewmx], [Enable visualization via OpenGL. Enabled by default if QtOpenGL and glewmx are available.])],
    [if test "$withval" = "yes"; then glewmx="yes"; else glewmx="no "; fi], [glewmx="yes"])
if test "$glewmx" = "yes"; then
    if test "$qtopengl" = "yes"; then
        PKG_CHECK_MODULES([libglewmx], [glewmx >= 1.6.0], [],
            [glewmx="no "
            AC_MSG_WARN([glewmx library not found:])
            AC_MSG_WARN([$libglewmx_PKG_ERRORS])
            AC_MSG_WARN([Disabled the view command.])])
    else
        glewmx="no "
    fi
fi
AC_DEFINE_UNQUOTED([WITH_GLEWMX], [`if test "$glewmx" = "yes"; then echo "1"; else echo "0"; fi`], [Use glewmx?])
AM_CONDITIONAL([WITH_GLEWMX], [test "$glewmx" = "yes"])
AC_ARG_WITH([equalizer],
    [AS_HELP_STRING([--with-equalizer], [Enable multi-display OpenGL support. Enabled by default if Equalizer is available.])],
    [if test "$withval" = "yes"; then equalizer="yes"; else equalizer="no "; fi], [equalizer="yes"])
if test "$equalizer" = "yes"; then
    if test "$qtopengl" = "yes" -a "$glewmx" = "yes"; then
        PKG_CHECK_MODULES([libequalizer], [Equalizer >= 1.0.0], [],
            [equalizer="no "
            AC_MSG_WARN([Equalizer library not found:])
            AC_MSG_WARN([$libequalizer_PKG_ERRORS])
            AC_MSG_WARN([Disabled multi-display support in the view command.])])
    else
        equalizer="no "
    fi
fi
AC_DEFINE_UNQUOTED([WITH_EQUALIZER], [`if test "$equalizer" = "yes"; then echo "1"; else echo "0"; fi`], [Use Equalizer?])
AM_CONDITIONAL([WITH_EQUALIZER], [test "$equalizer" = "yes"])
AC_ARG_WITH([gls],
    [AS_HELP_STRING([--with-gls], [Enable stereoscopic-3D OpenGL support. Enabled by default if libgls is available.])],
    [if test "$withval" = "yes"; then gls="yes"; else gls="no "; fi], [gls="yes"])
if test "$gls" = "yes"; then
    if test "$qtopengl" = "yes" -a "$glewmx" = "yes"; then
        PKG_CHECK_MODULES([libgls], [gls >= 1.0.0], [],
            [gls="no "
            AC_MSG_WARN([gls library not found:])
            AC_MSG_WARN([$libgls_PKG_ERRORS])
            AC_MSG_WARN([Disabled stereoscopic-3D support in the view command.])])
    else
        gls="no "
    fi
fi
AC_DEFINE_UNQUOTED([WITH_GLS], [`if test "$gls" = "yes"; then echo "1"; else echo "0"; fi`], [Use libgls?])
AM_CONDITIONAL([WITH_GLS], [test "$gls" = "yes"])
dnl We need to explicitly link with OpenGL since we use its functions directly.
dnl However, this is most probably not necessary on Windows and Mac OS X (for
dnl now), so we just use a pkg-config check (which usually only works on Linux)
dnl and ignore a failure.
PKG_CHECK_MODULES([libgl], [gl >= 0.0], [HAVE_LIBGL=1], [HAVE_LIBGL=0])

dnl bash-completion
AC_ARG_WITH([bashcompletion],
    [AS_HELP_STRING([--with-bash-completion], [Enable bash-completion support. Enabled by default if bash-completion is available.])],
    [if test "$withval" = "yes"; then bashcompletion="yes"; else bashcompletion="no "; fi], [bashcompletion="yes"])
if test "$bashcompletion" = "yes"; then
    AC_MSG_CHECKING([for bash-completion])
    PKG_CHECK_EXISTS([bash-completion >= 0.0], [AC_MSG_RESULT([yes])],
        [AC_MSG_RESULT([no])
	bashcompletion="no "
        AC_MSG_WARN([bash-completion not found.])
        AC_MSG_WARN([Disabled installation of bash-completion script.])])
fi
AC_ARG_VAR([BASHCOMPLETIONDIR], [directory for bash completion script])
if test "$bashcompletion" = "yes" -a -z "$BASHCOMPLETIONDIR"; then
    BASHCOMPLETIONDIR="`$PKG_CONFIG --variable=completionsdir bash-completion`"
fi
AM_CONDITIONAL([WITH_BASHCOMPLETION], [test "$bashcompletion" = "yes"])


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
echo "Bash completion script:  " "$bashcompletion" "(Requires bash-completion)"
echo ""
echo "component-compute:       " "$muparser" "(Requires muParser)"
echo "from-csv, to-csv:        " "$csv" ""
echo "from-datraw, to-datraw:  " "$datraw" ""
echo "from-dcmtk:              " "$dcmtk" "(Requires DCMTK)"
echo "from-exr, to-exr:        " "$exr" "(Requires OpenEXR)"
echo "from-ffmpeg:             " "$ffmpeg" "(Requires FFmpeg)"
echo "from-gdal, to-gdal:      " "$gdal" "(Requires GDAL)"
echo "from-jpeg, to-jpeg:      " "$jpeg" "(Requires libjpeg)"
echo "from-magick, to-magick:  " "$magick" "(Requires GraphicsMagick or ImageMagick, see --with-magick-flavor)"
echo "from-mat, to-mat:        " "$mat" "(Requires matio)"
echo "from-netcdf, to-netcdf:  " "$netcdf" "(Requires NetCDF)"
echo "from-netpbm, to-netpbm:  " "$netpbm" "(Requires NetPBM)"
echo "from-pcd, to-pcd:        " "$pcd" "(Requires the Point Cloud Library IO module libpcl_io)"
echo "from-pfs, to-pfs:        " "$pfs" "(Requires pfstools)"
echo "from-ply, to-ply:        " "$ply" ""
echo "from-pmd:                " "$pmd" "(Requires the proprietary PMD SDK)"
echo "from-png, to-png:        " "$png" "(Requires libpng)"
echo "from-pvm, to-pvm:        " "$pvm" ""
echo "from-rat, to-rat:        " "$rat" ""
echo "from-raw, to-raw:        " "$raw" ""
echo "from-sndfile, to-sndfile:" "$sndfile" "(Requires libsndfile)"
echo "from-teem, to-teem:      " "$teem" "(Requires libteem)"
echo "gui:                     " "$qt" "(Requires Qt)"
echo "view:                    " "$glewmx" "(Requires QtOpenGL and glewmx)"
echo "view with multi-display: " "$equalizer" "(Additionally requires Equalizer)"
echo "view with stereo-3D:     " "$gls" "(Additionally requires libgls)"
echo ""
