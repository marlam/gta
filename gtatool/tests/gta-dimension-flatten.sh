#!/usr/bin/env bash

# Copyright (C) 2010, 2011, 2013
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA dimension-flatten --help 2> "$TMPD"/help.txt

$GTA create -d 3,5 -c uint8 -v 42 "$TMPD"/a.gta
$GTA create -d 15 -c uint8 -v 42 "$TMPD"/b.gta

$GTA dimension-flatten < "$TMPD"/a.gta > "$TMPD"/xb.gta
cmp "$TMPD"/xb.gta "$TMPD"/b.gta

$GTA create -n5 > "$TMPD"/empty0.gta
$GTA create -d 100 -n5 > "$TMPD"/empty1.gta
$GTA create -d 10,10 -n5 > "$TMPD"/empty2.gta
$GTA create -c uint8 -n5 > "$TMPD"/empty3.gta
$GTA dimension-flatten "$TMPD"/empty0.gta > "$TMPD"/xempty0.gta
cmp "$TMPD"/empty0.gta "$TMPD"/xempty0.gta
$GTA dimension-flatten "$TMPD"/empty2.gta > "$TMPD"/xempty1.gta
cmp "$TMPD"/empty1.gta "$TMPD"/xempty1.gta
$GTA dimension-flatten "$TMPD"/empty3.gta > "$TMPD"/xempty3.gta
cmp "$TMPD"/empty3.gta "$TMPD"/xempty3.gta
$GTA dimension-flatten -p "$TMPD"/empty2.gta > "$TMPD"/x.gta

rm -r "$TMPD"
