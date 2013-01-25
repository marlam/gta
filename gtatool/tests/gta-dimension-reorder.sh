#!/usr/bin/env bash

# Copyright (C) 2010, 2011, 2013
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA dimension-reorder --help 2> "$TMPD"/help.txt

$GTA create -d 5,7,3 -c uint8 -v 42 "$TMPD"/a.gta
$GTA create -d 3,7,5 -c uint8 -v 42 "$TMPD"/b.gta
$GTA compress "$TMPD"/b.gta > "$TMPD"/zb.gta

$GTA dimension-reorder "$TMPD"/a.gta > "$TMPD"/c.gta
cmp "$TMPD"/c.gta "$TMPD"/a.gta

$GTA dimension-reorder -i 0,1,2 "$TMPD"/a.gta > "$TMPD"/d.gta
cmp "$TMPD"/d.gta "$TMPD"/a.gta

$GTA dimension-reorder -i 2,1,0 "$TMPD"/a.gta > "$TMPD"/e.gta
cmp "$TMPD"/e.gta "$TMPD"/b.gta

$GTA dimension-reorder -i 2,1,0 "$TMPD"/b.gta > "$TMPD"/f.gta
cmp "$TMPD"/f.gta "$TMPD"/a.gta

$GTA dimension-reorder -i 2,1,0 "$TMPD"/zb.gta > "$TMPD"/g.gta
cmp "$TMPD"/g.gta "$TMPD"/a.gta

cat "$TMPD"/b.gta | $GTA dimension-reorder -i 2,1,0 > "$TMPD"/h.gta
cmp "$TMPD"/h.gta "$TMPD"/a.gta

rm -r "$TMPD"
