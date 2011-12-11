#!/usr/bin/env bash

# Copyright (C) 2011
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

$GTA create -d 10,10 -c uint8,uint16,uint32,uint64 -v 0,1,2,3 "$TMPD"/abcd.gta
$GTA create -d 10,10 -c uint16,uint64 -v 1,3 "$TMPD"/bd.gta
$GTA create -d 10,10 -c uint8,uint16 -v 0,1 "$TMPD"/ab.gta

$GTA component-merge "$TMPD"/a.gta "$TMPD"/b.gta "$TMPD"/c.gta "$TMPD"/d.gta > "$TMPD"/xabcd.gta
$GTA component-merge "$TMPD"/b.gta "$TMPD"/d.gta > "$TMPD"/xbd.gta
$GTA component-merge "$TMPD"/a.gta "$TMPD"/b.gta > "$TMPD"/xab.gta

cmp "$TMPD"/abcd.gta "$TMPD"/xabcd.gta
cmp "$TMPD"/bd.gta "$TMPD"/xbd.gta
cmp "$TMPD"/ab.gta "$TMPD"/xab.gta

rm -r "$TMPD"
