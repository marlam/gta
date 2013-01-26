#!/usr/bin/env bash

# Copyright (C) 2010, 2011, 2013
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c uint8 -v 0 "$TMPD"/0.gta
$GTA create -d 10,10 -c uint8 -v 0 "$TMPD"/1.gta
$GTA create -d 10,10 -c uint8 -v 0 "$TMPD"/2.gta
$GTA create -d 10,10 -c uint8 -v 0 -n 3 "$TMPD"/012.gta

$GTA stream-merge --help 2> "$TMPD"/help.txt

$GTA stream-merge < "$TMPD"/0.gta > "$TMPD"/x0.gta
cmp "$TMPD"/x0.gta "$TMPD"/0.gta

$GTA stream-merge "$TMPD"/0.gta > "$TMPD"/y0.gta
cmp "$TMPD"/y0.gta "$TMPD"/0.gta

$GTA stream-merge "$TMPD"/0.gta "$TMPD"/1.gta "$TMPD"/2.gta > "$TMPD"/x012.gta
cmp "$TMPD"/x012.gta "$TMPD"/012.gta

$GTA create -d 10 -n4 > "$TMPD"/empty0.gta
$GTA create -d 10 -n2 > "$TMPD"/empty1.gta
$GTA create -c uint8 -n4 > "$TMPD"/empty2.gta
$GTA create -c uint8 -n2 > "$TMPD"/empty3.gta
$GTA stream-merge "$TMPD"/empty1.gta "$TMPD"/empty1.gta > "$TMPD"/xempty0.gta
cmp "$TMPD"/empty0.gta "$TMPD"/xempty0.gta
$GTA stream-merge "$TMPD"/empty3.gta "$TMPD"/empty3.gta > "$TMPD"/xempty2.gta
cmp "$TMPD"/empty2.gta "$TMPD"/xempty2.gta

rm -r "$TMPD"
