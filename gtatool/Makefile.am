# Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

SUBDIRS = src tests
ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = m4/gnulib-cache.m4

# Create the ChangeLog file from the git log
dist-hook:
	if test -d "$(srcdir)/../.git" && type git > /dev/null 2>&1; then \
		(cd "$(srcdir)/.." && git log --date=iso --stat gtatool) > "$(distdir)/ChangeLog"; \
	fi
