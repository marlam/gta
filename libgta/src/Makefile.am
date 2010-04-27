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
