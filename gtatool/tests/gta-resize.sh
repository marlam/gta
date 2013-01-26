#!/usr/bin/env bash

# Copyright (C) 2010, 2011, 2013
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c uint8 -v 42 "$TMPD"/a.gta
$GTA create -d 10,10 -c uint8 -v  0 "$TMPD"/b.gta
$GTA create -d 20,20 -c uint8 -v 42 "$TMPD"/c.gta

$GTA resize -d 20,20 -v 42 "$TMPD"/a.gta > "$TMPD"/d.gta
cmp "$TMPD"/d.gta "$TMPD"/c.gta

$GTA resize -d 10,10 -v 0 "$TMPD"/c.gta > "$TMPD"/e.gta
cmp "$TMPD"/e.gta "$TMPD"/a.gta

$GTA resize -d 10,10 -i -20,-20 -v 0 "$TMPD"/a.gta > "$TMPD"/f.gta
cmp "$TMPD"/f.gta "$TMPD"/b.gta

$GTA create -d 1,1 -n2 > "$TMPD"/empty0.gta
$GTA create -d 2,2 -n2 > "$TMPD"/empty1.gta
$GTA resize -d 2,2 "$TMPD"/empty0.gta > "$TMPD"/xempty1.gta
cmp "$TMPD"/empty1.gta "$TMPD"/xempty1.gta

rm -r "$TMPD"
