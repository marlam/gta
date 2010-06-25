# Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

EXTRA_DIST = doxyfile.in

dist_doc_DATA = \
	example-basic.c example-basic.cpp \
	example-tags.c example-tags.cpp

BUILT_SOURCES =
nobase_nodist_doc_DATA =
CLEANFILES =

if BUILD_REF

BUILT_SOURCES += reference/index.html
nobase_nodist_doc_DATA += reference/*
CLEANFILES += reference/*

reference/index.html: doxyfile $(top_srcdir)/src/gta/gta.h $(top_srcdir)/src/gta/gta.hpp
	$(DOXYGEN) doxyfile

endif
