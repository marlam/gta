#!/usr/bin/env bash

# Copyright (C) 2010, 2011
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA dimension-reorder --help 2> "$TMPD"/help.txt

$GTA create -d 5,3 -c uint8 -v 42 "$TMPD"/a.gta

$GTA dimension-reorder "$TMPD"/a.gta > "$TMPD"/xa.gta
cmp "$TMPD"/xa.gta "$TMPD"/a.gta

rm -r "$TMPD"
