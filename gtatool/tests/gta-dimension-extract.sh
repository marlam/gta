#!/usr/bin/env bash

# Copyright (C) 2010, 2011
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA dimension-extract --help 2> "$TMPD"/help.txt

$GTA create -d 3,5 -c uint8 -v 42 "$TMPD"/a.gta
$GTA create -d 3,5,7 -c uint8 -v 42 "$TMPD"/b.gta

$GTA dimension-extract -d 2 -i 0 < "$TMPD"/b.gta > "$TMPD"/xa.gta
cmp "$TMPD"/xa.gta "$TMPD"/a.gta

rm -r "$TMPD"
