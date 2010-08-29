# Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

TESTS = \
	gta-compress.sh \
	gta-create.sh \
	gta-extract.sh \
	gta-fill.sh \
	gta-help.sh \
	gta-info.sh \
	gta-merge.sh \
	gta-resize.sh \
	gta-set.sh \
	gta-tag.sh \
	gta-uncompress.sh \
	gta-version.sh

EXTRA_DIST = $(TESTS)

if VALGRIND_TESTS
VALGRIND_CMD = $(VALGRIND) --quiet --log-fd=2 --error-exitcode=1 \
	--tool=memcheck --leak-check=full --leak-resolution=high --show-reachable=no
else
VALGRIND_CMD =
endif

TESTS_ENVIRONMENT = GTA="$(VALGRIND_CMD) $(top_builddir)/src/gta"
