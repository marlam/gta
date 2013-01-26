#!/usr/bin/env bash

# Copyright (C) 2011, 2013
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c uint8  -v 0 "$TMPD"/a.gta
$GTA create -d 10,10 -c uint16 -v 1 "$TMPD"/b.gta
$GTA create -d 10,10 -c uint32 -v 2 "$TMPD"/c.gta
$GTA create -d 10,10 -c uint64 -v 3 "$TMPD"/d.gta

$GTA create -d 10,10 -c uint8,uint16,uint32,uint64 -v 0,1,2,3 "$TMPD"/all.gta
$GTA stream-merge "$TMPD"/a.gta "$TMPD"/b.gta "$TMPD"/c.gta "$TMPD"/d.gta > "$TMPD"/alls.gta
$GTA stream-merge "$TMPD"/b.gta "$TMPD"/c.gta > "$TMPD"/12.gta

$GTA component-split "$TMPD"/all.gta > "$TMPD"/xalls.gta
$GTA component-split -d 0,3 "$TMPD"/all.gta > "$TMPD"/x12.gta

cmp "$TMPD"/alls.gta "$TMPD"/xalls.gta
cmp "$TMPD"/12.gta "$TMPD"/x12.gta

$GTA create -d 10 -n5 > "$TMPD"/empty0.gta
$GTA create -c uint8,uint8 -n5 > "$TMPD"/empty1.gta
$GTA create -c uint8 -n10 > "$TMPD"/empty2.gta
$GTA component-split "$TMPD"/empty0.gta > "$TMPD"/xdevnull.gta
cmp /dev/null "$TMPD"/xdevnull.gta
$GTA component-split "$TMPD"/empty1.gta > "$TMPD"/xempty2.gta
cmp "$TMPD"/empty2.gta "$TMPD"/xempty2.gta

rm -r "$TMPD"
