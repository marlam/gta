dnl Process this file with autoconf to produce a configure script.

dnl configure.ac
dnl
dnl This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
dnl (GTAs).
dnl
dnl Copyright (C) 2010, 2011
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
AC_INIT([gtatool], [0.9.3], [gta-list@nongnu.org], [gtatool], [http://gta.nongnu.org/])
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
gl_EARLY
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
else
    MAKENSIS=":"
fi
AM_CONDITIONAL([HAVE_MAKENSIS], [test "$MAKENSIS" != ":"])
AM_CONDITIONAL([W32], [test "$w32" = "yes"])
if test "$w32" = "yes"; then W32=1; else W32=0; fi
AC_DEFINE_UNQUOTED([W32], [$W32], [Use W32 API?])

dnl Gnulib initialization
gl_INIT

dnl Valgrind
AC_ARG_ENABLE([valgrind-tests],
    [AS_HELP_STRING([--enable-valgrind-tests], [Use valgrind when running tests. Enabled by default if valgrind is available.])],
    [if test "$enableval" = "yes"; then valgrind_tests="yes"; else valgrind_tests="no "; fi], [valgrind_tests="yes"])
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

dnl Feature checks needed by the base module
LIB_ICONV=
AC_SUBST([LIB_ICONV])
AC_CHECK_FUNCS([iconv_open], [],
    [AC_LIB_HAVE_LINKFLAGS([iconv], [], [#include <iconv.h>], [iconv_open(0, 0);])
     if test "$HAVE_LIBICONV" != "yes"; then
         AC_MSG_ERROR([libiconv not found])
     fi
     LIB_ICONV="$LTLIBICONV"])
AC_CHECK_DECLS([optreset], [#include <getopt.h>])
AC_CHECK_FUNCS([backtrace fseeko ftello link mmap nl_langinfo sigaction vasprintf wcwidth])
AC_DEFINE_UNQUOTED([NDEBUG], [1], [Define to 1 to disable assertions and other debugging checks.])

dnl The GTA library
AC_LIB_HAVE_LINKFLAGS([gta], [],
    [#include <gta/gta.h>
     #if GTA_VERSION_NUMBER < 0x000903
     # error GTA version too old
     #endif],
    [gta_version(0, 0, 0);])
if test "$HAVE_LIBGTA" != "yes"; then
    AC_MSG_ERROR([libgta >= 0.9.3 not found])
fi

dnl The muParser library
AC_ARG_WITH([muparser],
    [AS_HELP_STRING([--with-muparser], [Enable the component-compute command. Enabled by default if libmuparser is available.])],
    [if test "$withval" = "yes"; then muparser="yes"; else muparser="no "; fi], [muparser="yes"])
if test "$muparser" = "yes"; then
    AC_CHECK_HEADER([muParser/muParser.h], [HAVE_MUPARSER_INCSUBDIR=yes], [HAVE_MUPARSER_INCSUBDIR=no])
    if test "$HAVE_MUPARSER_INCSUBDIR" = "yes"; then
        gl_ABSOLUTE_HEADER([muParser/muParser.h])
        MUPARSER_INCLUDE_DIR=`echo $gl_cv_absolute_muParser_muParser_h \
            | sed -e 's/\/muParser.h$//' -e 's/^\/\/\//\//'`
        CPPFLAGS="$CPPFLAGS -isystem $MUPARSER_INCLUDE_DIR"
    fi
    AC_LIB_FROMPACKAGE([muparser], [muparser])
    AC_LIB_HAVE_LINKFLAGS([muparser], [], [#include <muParser.h>], [mu::Parser p;])
    if test "$HAVE_LIBMUPARSER" != "yes"; then
        muparser="no "
        AC_MSG_WARN([Disabled the component-compute command because libmuparser was not found.])
    fi
fi
AC_DEFINE_UNQUOTED([WITH_MUPARSER], [`if test "$muparser" = "yes"; then echo "1"; else echo "0"; fi`], [Use muparser?])
AM_CONDITIONAL([WITH_MUPARSER], [test "$muparser" = "yes"])

dnl NetPBM
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
        AC_MSG_WARN([Disabled NetPBM import/export because libnetpbm was not found.])
    fi
fi
AC_DEFINE_UNQUOTED([WITH_NETPBM], [`if test "$netpbm" = "yes"; then echo "1"; else echo "0"; fi`], [Use netpbm?])
AM_CONDITIONAL([WITH_NETPBM], [test "$netpbm" = "yes"])

dnl ImageMagick
AC_ARG_WITH([magick],
    [AS_HELP_STRING([--with-magick], [Enable ImageMagick import/export. Enabled by default if libMagick++ is available.])],
    [if test "$withval" = "yes"; then magick="yes"; else magick="no "; fi], [magick="yes"])
if test "$magick" = "yes"; then
    AC_CHECK_HEADER([ImageMagick/magick/magick-config.h], [HAVE_MAGICK_INCSUBDIR=yes], [HAVE_MAGICK_INCSUBDIR=no])
    if test "$HAVE_MAGICK_INCSUBDIR" = "yes"; then
        gl_ABSOLUTE_HEADER([ImageMagick/magick/magick-config.h])
        MAGICK_INCLUDE_DIR=`echo $gl_cv_absolute_ImageMagick_magick_magick_config_h \
            | sed -e 's/\/magick\/magick-config.h$//' -e 's/^\/\/\//\//'`
        CPPFLAGS="$CPPFLAGS -isystem $MAGICK_INCLUDE_DIR"
    fi
    AC_LIB_FROMPACKAGE([Magick++], [magick])
    AC_LIB_HAVE_LINKFLAGS([Magick++], [], [#include <Magick++.h>], [Magick::InitializeMagick(0);])
    if test "$HAVE_LIBMAGICK__" != "yes"; then
        magick="no "
        AC_MSG_WARN([Disabled ImageMagick import/export because libMagick++ was not found.])
    fi
fi
AC_DEFINE_UNQUOTED([WITH_MAGICK], [`if test "$magick" = "yes"; then echo "1"; else echo "0"; fi`], [Use magick?])
AM_CONDITIONAL([WITH_MAGICK], [test "$magick" = "yes"])

dnl PFS
AC_ARG_WITH([pfs],
    [AS_HELP_STRING([--with-pfs], [Enable PFS import/export. Enabled by default if libpfs-1.2 is available.])],
    [if test "$withval" = "yes"; then pfs="yes"; else pfs="no "; fi], [pfs="yes"])
if test "$pfs" = "yes"; then
    AC_LIB_FROMPACKAGE([pfs-1.2], [pfs])
    AC_LIB_HAVE_LINKFLAGS([pfs-1.2], [], [#include <pfs-1.2/pfs.h>], [pfs::FrameFile ff(NULL, NULL);])
    if test "$HAVE_LIBPFS_1_2" != "yes"; then
        pfs="no "
        AC_MSG_WARN([Disabled PFS import/export because libpfs-1.2 was not found.])
    fi
fi
AC_DEFINE_UNQUOTED([WITH_PFS], [`if test "$pfs" = "yes"; then echo "1"; else echo "0"; fi`], [Use pfs?])
AM_CONDITIONAL([WITH_PFS], [test "$pfs" = "yes"])

dnl OpenEXR
AC_ARG_WITH([exr],
    [AS_HELP_STRING([--with-exr], [Enable OpenEXR import/export. Enabled by default if libIlmImf is available.])],
    [if test "$withval" = "yes"; then exr="yes"; else exr="no "; fi], [exr="yes"])
if test "$exr" = "yes"; then
    dnl FIXME: Despite AC_LANG([C++]), AC_CHECK_HEADER uses the C compiler (why??).
    dnl We cannot test for OpenEXR/ImfVersion.h because that contains a namespace.
    dnl We're testing for a header that is safe even for C.
    AC_CHECK_HEADER([OpenEXR/OpenEXRConfig.h], [HAVE_OPENEXR_INCSUBDIR=yes], [HAVE_OPENEXR_INCSUBDIR=no])
    if test "$HAVE_OPENEXR_INCSUBDIR" = "yes"; then
        gl_ABSOLUTE_HEADER([OpenEXR/OpenEXRConfig.h])
        OPENEXR_INCLUDE_DIR=`echo $gl_cv_absolute_OpenEXR_OpenEXRConfig_h \
            | sed -e 's/\/OpenEXRConfig.h$//' -e 's/^\/\/\//\//'`
        CPPFLAGS="$CPPFLAGS -isystem $OPENEXR_INCLUDE_DIR"
    fi
    AC_LIB_FROMPACKAGE([IlmImf], [exr])
    AC_LIB_HAVE_LINKFLAGS([IlmImf], [], [#include <ImfVersion.h>], [Imf::getVersion(0)])
    if test "$HAVE_LIBILMIMF" != "yes"; then
        exr="no "
        AC_MSG_WARN([Disabled OpenEXR import/export because libIlmImf was not found.])
    fi
fi
AC_DEFINE_UNQUOTED([WITH_EXR], [`if test "$exr" = "yes"; then echo "1"; else echo "0"; fi`], [Use exr?])
AM_CONDITIONAL([WITH_EXR], [test "$exr" = "yes"])

dnl GDAL
AC_ARG_WITH([gdal],
    [AS_HELP_STRING([--with-gdal], [Enable GDAL import/export. Enabled by default if libgdal is available.])],
    [if test "$withval" = "yes"; then gdal="yes"; else gdal="no "; fi], [gdal="yes"])
if test "$gdal" = "yes"; then
    AC_CHECK_HEADER([gdal/gdal.h], [HAVE_GDAL_INCSUBDIR=yes], [HAVE_GDAL_INCSUBDIR=no])
    if test "$HAVE_GDAL_INCSUBDIR" = "yes"; then
        gl_ABSOLUTE_HEADER([gdal/gdal.h])
        GDAL_INCLUDE_DIR=`echo $gl_cv_absolute_gdal_gdal_h \
            | sed -e 's/\/gdal.h$//' -e 's/^\/\/\//\//'`
        CPPFLAGS="$CPPFLAGS -isystem $GDAL_INCLUDE_DIR"
    fi
    AC_LIB_FROMPACKAGE([gdal], [gdal])
    AC_LIB_HAVE_LINKFLAGS([gdal], [], [#include <gdal.h>], [GDALAllRegister();])
    dnl On Debian, libgdal.so is called libgdal1.?.0.so.
    if test "$HAVE_LIBGDAL" != "yes"; then
        AC_LIB_FROMPACKAGE([gdal1.7.0], [gdal])
        AC_LIB_HAVE_LINKFLAGS([gdal1.7.0], [], [#include <gdal.h>], [GDALAllRegister();])
        HAVE_LIBGDAL="$HAVE_LIBGDAL1_7_0"
        if test "$HAVE_LIBGDAL" = "yes"; then
            LIBGDAL="$LIBGDAL1_7_0"
            LTLIBGDAL="$LTLIBGDAL1_7_0"
	    AC_DEFINE_UNQUOTED([HAVE_LIBGDAL], [1], [Define if you have the libgdal library.])
        fi
    fi
    if test "$HAVE_LIBGDAL" != "yes"; then
        AC_LIB_FROMPACKAGE([gdal1.6.0], [gdal])
        AC_LIB_HAVE_LINKFLAGS([gdal1.6.0], [], [#include <gdal.h>], [GDALAllRegister();])
        HAVE_LIBGDAL="$HAVE_LIBGDAL1_6_0"
        if test "$HAVE_LIBGDAL" = "yes"; then
            LIBGDAL="$LIBGDAL1_6_0"
            LTLIBGDAL="$LTLIBGDAL1_6_0"
	    AC_DEFINE_UNQUOTED([HAVE_LIBGDAL], [1], [Define if you have the libgdal library.])
        fi
    fi
    if test "$HAVE_LIBGDAL" != "yes"; then
        AC_LIB_FROMPACKAGE([gdal1.5.0], [gdal])
        AC_LIB_HAVE_LINKFLAGS([gdal1.5.0], [], [#include <gdal.h>], [GDALAllRegister();])
        HAVE_LIBGDAL="$HAVE_LIBGDAL1_5_0"
        if test "$HAVE_LIBGDAL" = "yes"; then
            LIBGDAL="$LIBGDAL1_5_0"
            LTLIBGDAL="$LTLIBGDAL1_5_0"
	    AC_DEFINE_UNQUOTED([HAVE_LIBGDAL], [1], [Define if you have the libgdal library.])
        fi
    fi
    if test "$HAVE_LIBGDAL" != "yes"; then
        gdal="no "
        AC_MSG_WARN([Disabled GDAL import/export because libgdal was not found.])
    fi
fi
AC_DEFINE_UNQUOTED([WITH_GDAL], [`if test "$gdal" = "yes"; then echo "1"; else echo "0"; fi`], [Use gdal?])
AM_CONDITIONAL([WITH_GDAL], [test "$gdal" = "yes"])

dnl DCMTK
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
        AC_MSG_WARN([Disabled DCMTK import because libdcmimgle was not found.])
    fi
fi
AC_DEFINE_UNQUOTED([WITH_DCMTK], [`if test "$dcmtk" = "yes"; then echo "1"; else echo "0"; fi`], [Use dcmtk?])
AM_CONDITIONAL([WITH_DCMTK], [test "$dcmtk" = "yes"])

dnl MATIO
AC_ARG_WITH([matio],
    [AS_HELP_STRING([--with-matio], [Enable MATLAB import/export. Enabled by default if libmatio is available.])],
    [if test "$withval" = "yes"; then mat="yes"; else mat="no "; fi], [mat="yes"])
if test "$mat" = "yes"; then
    AC_LIB_FROMPACKAGE([matio], [matio])
    case "${target}" in
    *-*-mingw32*)
        AC_LIB_HAVE_LINKFLAGS([matio], [z], [#include <matio.h>], [Mat_Open(0, 0);])
        ;;
    *)
        AC_LIB_HAVE_LINKFLAGS([matio], [], [#include <matio.h>], [Mat_Open(0, 0);])
        ;;
    esac
    if test "$HAVE_LIBMATIO" != "yes"; then
        mat="no "
        AC_MSG_WARN([Disabled MATLAB import/export because libmatio was not found.])
    fi
fi
AC_DEFINE_UNQUOTED([WITH_MAT], [`if test "$mat" = "yes"; then echo "1"; else echo "0"; fi`], [Use mat?])
AM_CONDITIONAL([WITH_MAT], [test "$mat" = "yes"])

dnl Qt
AC_ARG_WITH([qt],
    [AS_HELP_STRING([--with-qt], [Enable Qt GUI. Enabled by default if Qt >= 4.4.3 is available.])],
    [if test "$withval" = "yes"; then qt="yes"; else qt="no "; fi], [qt="yes"])
if test "$qt" = "yes"; then
    RCC=""
    AC_ARG_VAR([RCC], [rcc command])
    AC_CHECK_TOOLS([RCC], [rcc-qt4 rcc])
    MOC=""
    AC_ARG_VAR([MOC], [moc command])
    AC_CHECK_TOOLS([MOC], [moc-qt4 moc])
    AC_CHECK_HEADER([qt4/Qt/qconfig.h], [HAVE_QTINC=yes], [HAVE_QTINC=no])
    if test "$HAVE_QTINC" = "yes"; then
        gl_ABSOLUTE_HEADER([qt4/Qt/qconfig.h])
        QT_INCLUDE_DIR=`echo $gl_cv_absolute_qt4_Qt_qconfig_h | sed -e 's/\/Qt\/qconfig.h$//' -e 's/^\/\/\//\//'`
    else
        AC_CHECK_HEADER([Qt/qconfig.h], [HAVE_QTINC=yes], [HAVE_QTINC=no])
        if test "$HAVE_QTINC" = "yes"; then
            gl_ABSOLUTE_HEADER([Qt/qconfig.h])
            QT_INCLUDE_DIR=`echo $gl_cv_absolute_Qt_qconfig_h | sed -e 's/\/Qt\/qconfig.h$//' -e 's/^\/\/\//\//'`
        fi
    fi
    if test "$RCC" != ":" -a "$MOC" != ":" -a "$HAVE_QTINC" = "yes"; then
        CPPFLAGS="$CPPFLAGS -isystem $QT_INCLUDE_DIR -isystem $QT_INCLUDE_DIR/Qt -isystem $QT_INCLUDE_DIR/QtCore -isystem $QT_INCLUDE_DIR/QtGui"
        AC_LIB_FROMPACKAGE([QtGui], [qt])
        case "${target}" in
        *-*-mingw32*)
            AC_LIB_HAVE_LINKFLAGS([QtGui], [gdi32 comdlg32 oleaut32 imm32 winmm winspool QtCore kernel32 user32 shell32 uuid ole32 advapi32 ws2_32 png z],
                [#include <QApplication>
                 #if (QT_VERSION < QT_VERSION_CHECK(4, 4, 3))
                 # error Qt version too old
                 #endif],
                [QApplication::exec();])
            ;;
        *)
	    AC_LIB_HAVE_LINKFLAGS([QtGui], [QtCore],
                [#include <QApplication>
                 #if (QT_VERSION < QT_VERSION_CHECK(4, 4, 3))
                 # error Qt version too old
                 #endif],
                [QApplication::exec();])
            ;;
        esac
    fi
    if test "$HAVE_LIBQTGUI" != "yes"; then
        qt="no "
        AC_MSG_WARN([Disabled Qt GUI because Qt >= 4.4.3 was not found.])
    fi
fi
AC_DEFINE_UNQUOTED([WITH_QT], [`if test "$qt" = "yes"; then echo "1"; else echo "0"; fi`], [Use Qt?])
AM_CONDITIONAL([WITH_QT], [test "$qt" = "yes"])

dnl Output
AC_CONFIG_FILES([Makefile	\
	src/Makefile		\
	src/base/Makefile	\
	src/cmds/Makefile	\
	doc/Makefile		\
	tests/Makefile		\
	])
AC_OUTPUT

echo ""
echo "Dynamic modules:       " "$dynamic_modules" "(Requires system support)"
echo ""
echo "component-compute:     " "$muparser" "(Requires muParser, Debian package: libmuparser-dev)"
echo "from-netpbm, to-netpbm:" "$netpbm" "(Requires NetPBM, Debian package: libnetpbm10-dev)"
echo "from-magick, to-magick:" "$magick" "(Requires ImageMagick, Debian package: libmagick++-dev)"
echo "from-pfs, to-pfs:      " "$pfs" "(Requires pfstools, Debian package: libpfs-dev)"
echo "from-exr, to-exr:      " "$exr" "(Requires OpenEXR, Debian package:libopenexr-dev)"
echo "from-gdal, to-gdal:    " "$gdal" "(Requires GDAL, Debian package: libgdal-dev)"
echo "from-dcmtk:            " "$dcmtk" "(Requires DCMTK, Debian package: libdcmtk1-dev)"
echo "from-mat, to-mat:      " "$mat" "(Requires matio, Debian package: libmatio-dev)"
echo "gui:                   " "$qt" "(Requires Qt, Debian package: libqt4-dev)"
echo ""
