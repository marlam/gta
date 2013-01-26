#!/usr/bin/env bash

# Copyright (C) 2010, 2011, 2013
# Martin Lambers <marlam@marlam.de>
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

$GTA create -d 10 -n3 > "$TMPD"/empty0.gta
$GTA create -d 10 -n1 > "$TMPD"/empty1.gta
$GTA create -c uint8 -n3 > "$TMPD"/empty2.gta
$GTA create -c uint8 -n1 > "$TMPD"/empty3.gta
$GTA stream-split "$TMPD"/e%3N.gta "$TMPD"/empty0.gta
cmp "$TMPD"/e000.gta "$TMPD"/empty1.gta
cmp "$TMPD"/e001.gta "$TMPD"/empty1.gta
cmp "$TMPD"/e002.gta "$TMPD"/empty1.gta
$GTA stream-split "$TMPD"/f%3N.gta "$TMPD"/empty2.gta
cmp "$TMPD"/f000.gta "$TMPD"/empty3.gta
cmp "$TMPD"/f001.gta "$TMPD"/empty3.gta
cmp "$TMPD"/f002.gta "$TMPD"/empty3.gta

rm -r "$TMPD"
