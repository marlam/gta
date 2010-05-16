# Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

SUBDIRS = base cmds
bin_PROGRAMS = gta
gta_SOURCES = main.cpp cmds.h cmds.cpp lib.h lib.cpp
AM_CPPFLAGS = -I$(top_srcdir)/src/base -DPKGLIBDIR=\"$(pkglibdir)\"
gta_LDADD = $(top_builddir)/src/cmds/libbuiltin.la \
	$(top_builddir)/src/base/libbase.la \
	$(LTLIBGTA) $(LIB_DLOPEN) $(LIB_ICONV)
gta_LDFLAGS = -export-dynamic
if W32
gta_SOURCES += appicon.ico
.ico.o:
	echo "IDI_ICON1 ICON DISCARDABLE \"$<\"" | $(WINDRES) -o $@
endif
