# Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

check_PROGRAMS = \
	basic		\
	compression	\
	taglists	\
	filedescriptors	\
	endianness	\
	blocks		\
	elements

TESTS = $(check_PROGRAMS)

EXTRA_DIST = little-endian.gta big-endian.gta

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_builddir)/src

LIBS = $(top_builddir)/src/libgta.la

# Prevent libtool from building annoying wrapper scripts,
# which would prevent us to check with valgrind.
AM_LDFLAGS = -no-install

if VALGRIND_TESTS
TESTS_ENVIRONMENT = $(VALGRIND) --quiet --log-fd=2 --error-exitcode=1 \
	--tool=memcheck --leak-check=full  --leak-resolution=high --show-reachable=no
endif
