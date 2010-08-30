#!/usr/bin/env bash

# Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c uint8 -v 0 -n 1 "$TMPD"/0.gta
$GTA create -d 10,10 -c uint8 -v 0 -n 1 "$TMPD"/1.gta
$GTA create -d 10,10 -c uint8 -v 0 -n 1 "$TMPD"/2.gta
$GTA create -d 10,10 -c uint8 -v 0 -n 3 "$TMPD"/012.gta

$GTA stream-split --help 2> "$TMPD"/help.txt

$GTA stream-split "$TMPD"/x%1N.gta < "$TMPD"/012.gta
cmp "$TMPD"/x0.gta "$TMPD"/0.gta
cmp "$TMPD"/x1.gta "$TMPD"/1.gta
cmp "$TMPD"/x2.gta "$TMPD"/2.gta

$GTA stream-split "$TMPD"/y%1N.gta "$TMPD"/012.gta
cmp "$TMPD"/y0.gta "$TMPD"/0.gta
cmp "$TMPD"/y1.gta "$TMPD"/1.gta
cmp "$TMPD"/y2.gta "$TMPD"/2.gta

rm -r "$TMPD"
