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

$GTA set -s "$TMPD"/b.gta "$TMPD"/a.gta > "$TMPD"/d.gta
cmp "$TMPD"/d.gta "$TMPD"/b.gta

$GTA set -s "$TMPD"/c.gta -i 100,100 "$TMPD"/b.gta > "$TMPD"/e.gta
cmp "$TMPD"/e.gta "$TMPD"/b.gta

rm -r "$TMPD"
