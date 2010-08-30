#!/usr/bin/env bash

# Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c uint8 -v 42 "$TMPD"/a.gta
$GTA create -d 10,10 -c uint8 -v  0 "$TMPD"/b.gta
$GTA create -d 20,20 -c uint8 -v 42 "$TMPD"/c.gta
$GTA fill -l 3,7 -h 12,16 -v 0 "$TMPD"/c.gta > "$TMPD"/d.gta

$GTA set -s "$TMPD"/b.gta "$TMPD"/a.gta > "$TMPD"/e.gta
cmp "$TMPD"/e.gta "$TMPD"/b.gta

$GTA set -s "$TMPD"/c.gta -i 100,-100 "$TMPD"/b.gta > "$TMPD"/f.gta
cmp "$TMPD"/f.gta "$TMPD"/b.gta

$GTA set -s "$TMPD"/c.gta -i -1,-1 "$TMPD"/b.gta > "$TMPD"/g.gta
cmp "$TMPD"/g.gta "$TMPD"/a.gta

$GTA set -s "$TMPD"/b.gta -i 3,7 "$TMPD"/c.gta > "$TMPD"/h.gta
cmp "$TMPD"/h.gta "$TMPD"/d.gta

rm -r "$TMPD"
