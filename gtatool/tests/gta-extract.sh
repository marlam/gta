#!/usr/bin/env bash

# Copyright (C) 2010, 2011
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c uint8 -v 42 "$TMPD"/a.gta
$GTA create -d 5,5 -c uint8 -v 117 "$TMPD"/b.gta
$GTA fill -l 3,3 -h 7,7 -v 117 < "$TMPD"/a.gta > "$TMPD"/c.gta

$GTA extract -l 3,3 -h 7,7 < "$TMPD"/c.gta > "$TMPD"/d.gta
cmp "$TMPD"/b.gta "$TMPD"/d.gta

rm -r "$TMPD"
