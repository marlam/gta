dnl Process this file with autoconf to produce a configure script.

dnl configure.ac
dnl
dnl This file is part of libgta, a library that implements the Generic Tagged
dnl Array (GTA) file format.
dnl
dnl Copyright (C) 2010, 2011, 2012
dnl Martin Lambers <marlam@marlam.de>
dnl
dnl Libgta is free software: you can redistribute it and/or modify it under the
dnl terms of the GNU Lesser General Public License as published by the Free
dnl Software Foundation, either version 2.1 of the License, or (at your option)
dnl any later version.
dnl
dnl Libgta is distributed in the hope that it will be useful, but WITHOUT ANY
dnl WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
dnl A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
dnl details.
dnl
dnl You should have received a copy of the GNU Lesser General Public License
dnl along with Libgta. If not, see <http://www.gnu.org/licenses/>.

dnl Autotools init stuff
AC_PREREQ([2.65])
AC_INIT([libgta], [1.0.4], [gta-list@nongnu.org], [libgta], [http://gta.nongnu.org/])
AC_CONFIG_SRCDIR([src/gta.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.11.1 color-tests silent-rules tar-pax -Wall -Wno-portability])
AM_SILENT_RULES([yes])
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_INSTALL
LT_PREREQ([2.2.6])
LT_INIT([win32-dll])

dnl Switches
AC_ARG_ENABLE([la-file],
    [AS_HELP_STRING([--enable-la-file], [Install the .la file. Disabled by default when a shared library is built.])],
    [if test "$enableval" = "yes"; then install_la_file="yes"; else install_la_file="no"; fi],
    [if test "$enable_shared" = "yes"; then install_la_file="no"; else install_la_file="yes"; fi])
AM_CONDITIONAL([INSTALL_LA_FILE], [test "$install_la_file" = "yes"])
AC_ARG_ENABLE([reference],
    [AS_HELP_STRING([--enable-reference], [Build the reference. Enabled by default if doxygen is available.])],
    [if test "$enableval" = "yes"; then build_ref="yes"; else build_ref="no"; fi], [build_ref="yes"])
AC_ARG_VAR([DOXYGEN], [Doxygen command])
if test "$build_ref" = "yes"; then
    AC_CHECK_PROGS([DOXYGEN], [doxygen], [:])
    if test "$DOXYGEN" = ":"; then
        build_ref="no"
        AC_MSG_WARN([Disabled building the reference because doxygen was not found.])
    fi
fi
AM_CONDITIONAL([BUILD_REF], [test "$build_ref" = "yes"])
AC_ARG_ENABLE([valgrind-tests],
    [AS_HELP_STRING([--enable-valgrind-tests], [Use valgrind when running tests. Disabled by default.])],
    [if test "$enableval" = "yes"; then valgrind_tests="yes"; else valgrind_tests="no"; fi], [valgrind_tests="no"])
AC_ARG_VAR([VALGRIND], [Valgrind command])
if test "$valgrind_tests" = "yes"; then
    AC_CHECK_PROGS([VALGRIND], [valgrind], [:])
    if test "$VALGRIND" = ":"; then
        valgrind_tests="no"
        AC_MSG_WARN([Disabled valgrind tests because valgrind was not found.])
    fi
fi
AM_CONDITIONAL([VALGRIND_TESTS], [test "$valgrind_tests" = "yes"])

dnl System
AC_SYS_LARGEFILE
AC_C_BIGENDIAN

dnl Compression libraries
AC_LIB_HAVE_LINKFLAGS([z], [], [#include <zlib.h>], [zlibVersion();])
if test "$HAVE_LIBZ" != "yes"; then
    AC_MSG_WARN([libz not found])
    AC_MSG_WARN([libz is provided by zlib; Debian package: libz-dev])
fi
AC_LIB_HAVE_LINKFLAGS([bz2], [],
    [#define BZ_IMPORT
     #include <bzlib.h>],
    [BZ2_bzlibVersion();])
if test "$HAVE_LIBBZ2" != "yes"; then
    AC_MSG_WARN([libbz2 not found])
    AC_MSG_WARN([libbz2 is provided by bzip2; Debian package: libbz2-dev])
fi
AC_LIB_HAVE_LINKFLAGS([lzma], [], [#include <lzma.h>], [lzma_version_string();])
if test "$HAVE_LIBLZMA" != "yes"; then
    AC_MSG_WARN([liblzma not found])
    AC_MSG_WARN([liblzma is provided by xz; Debian package: liblzma-dev])
fi
if test "$HAVE_LIBZ" != "yes" -o "$HAVE_LIBBZ2" != "yes" -o "$HAVE_LIBLZMA" != "yes"; then
    AC_MSG_ERROR([Required libraries were not found. See messages above.])
fi

dnl libgta package version
AC_SUBST([GTA_VERSION], [$PACKAGE_VERSION])
AC_SUBST([GTA_VERSION_MAJOR], [`echo $GTA_VERSION | sed -e 's/\(.*\)\..*\..*/\1/'`])
AC_SUBST([GTA_VERSION_MINOR], [`echo $GTA_VERSION | sed -e 's/.*\.\(.*\)\..*/\1/'`])
AC_SUBST([GTA_VERSION_PATCH], [`echo $GTA_VERSION | sed -e 's/.*\..*\.\(.*\)/\1/' -e 's/[[^0-9]].*//'`])
AC_SUBST([GTA_VERSION_NUMBER], [`printf "0x%02x%02x%02x" $GTA_VERSION_MAJOR $GTA_VERSION_MINOR $GTA_VERSION_PATCH`])

dnl libgta shared library version
dnl See http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info
dnl for a description of library versioning. In summary:
dnl Library code modified:            REVISION++
dnl Interfaces changed/added/removed: CURRENT++ REVISION=0
dnl Interfaces added:                 AGE++
dnl Interfaces removed:               AGE=0
AC_SUBST([LT_CURRENT], [0])
AC_SUBST([LT_REVISION], [2])
AC_SUBST([LT_AGE], [0])

dnl Sanity checks for type sizes
AC_CHECK_SIZEOF([int8_t])
AC_CHECK_SIZEOF([int])

dnl Output
AC_CONFIG_FILES([Makefile	\
	doc/Makefile		\
	doc/doxyfile		\
	src/Makefile		\
	src/gta/gta_version.h	\
	src/gta.pc		\
	tests/Makefile		\
	])
AC_OUTPUT
