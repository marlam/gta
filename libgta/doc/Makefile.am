# Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

EXTRA_DIST = doxyfile.in example.c example.cpp

doc_DATA =
nodist_doc_DATA =
nobase_nodist_doc_DATA =
CLEANFILES =

if BUILD_REF

doc_DATA += example.c example.cpp

DOXYFILES = \
	annotated.html				\
	classgta_1_1custom__io-members.html	\
	classgta_1_1custom__io.html		\
	classgta_1_1exception-members.html	\
	classgta_1_1exception.html		\
	classgta_1_1header-members.html		\
	classgta_1_1header.html			\
	classgta_1_1taglist-members.html	\
	classgta_1_1taglist.html		\
	doxygen.css				\
	doxygen.png				\
	example.c.html				\
	example.cpp.html			\
	files.html				\
	functions.html				\
	functions_func.html			\
	globals.html				\
	globals_enum.html			\
	globals_eval.html			\
	globals_func.html			\
	globals_type.html			\
	gta_8h.html				\
	gta_8hpp.html				\
	index.html				\
	namespacegta.html			\
	namespacemembers.html			\
	namespacemembers_enum.html		\
	namespacemembers_eval.html		\
	namespacemembers_func.html		\
	namespaces.html				\
	pages.html				\
	tab_b.gif				\
	tab_l.gif				\
	tab_r.gif				\
	tabs.css

REFDOXYFILES = $(addprefix reference/,$(DOXYFILES))

reference/index.html: doxyfile $(top_srcdir)/src/gta/gta.h $(top_srcdir)/src/gta/gta.hpp
	$(DOXYGEN) doxyfile

$(filter-out reference/index.html,$(REFDOXYFILES)):	reference/index.html

nobase_nodist_doc_DATA += $(REFDOXYFILES)

CLEANFILES += $(REFDOXYFILES)

endif
