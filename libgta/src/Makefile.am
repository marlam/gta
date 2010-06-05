# Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

lib_LTLIBRARIES = libgta.la
nobase_include_HEADERS = gta/gta_version.h gta/gta.h gta/gta.hpp
libgta_la_SOURCES = gta.c
libgta_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -no-undefined
libgta_la_LIBADD = $(LTLIBLZMA) $(LTLIBBZ2) $(LTLIBZ)
AM_CFLAGS = $(CFLAG_VISIBILITY)
AM_CPPFLAGS = -I$(top_builddir)/src
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gta.pc
DISTCLEANFILES = $(pkgconfig_DATA)

# When a shared libgta library is built, the libgta.la file causes unnecessary
# direct linking of the application with liblzma, libbz2, libz. This is bad
# because the application then unnecessarily depends on the .so versions of
# these libraries, even though it does not use their interfaces directly. Direct
# linking with these libraries is only necessary for the static libgta library.
#
# Pkg-config solves this by providing both a Libs and Libs.private field, where
# Libs.private is only used for static linking.
#
# As of 2010-04-29, libtool has no fix for this problem. Therefore we just
# remove the libgta.la file by default if a shared library is built. This can
# be overriden by a configure option (--enable-la-file).
if INSTALL_LA_FILE
else
install-exec-hook:
	rm $(libdir)/libgta.la
endif
