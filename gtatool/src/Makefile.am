# Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

SUBDIRS = base cmds
bin_PROGRAMS = gta
gta_SOURCES = main.cpp lib.h lib.cpp
AM_CPPFLAGS = -I$(top_srcdir)/src/base -DPKGLIBDIR=\"$(pkglibdir)\"
gta_LDADD = $(top_builddir)/src/cmds/libbuiltin.la	\
	$(top_builddir)/src/base/libbase.la		\
	$(LTLIBGTA) $(LIB_CLOCK_GETTIME) $(LIB_DLOPEN)
gta_LDFLAGS = -export-dynamic
