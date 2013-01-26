#!/usr/bin/env bash

# Copyright (C) 2011, 2013
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c int16,int32 -v 1,2 "$TMPD"/a.gta
$GTA create -d 10,10 -c int8,int16,int32 -v 0,1,2 "$TMPD"/b.gta
$GTA create -d 10,10 -c int8,int16,int32,int64 -v 0,1,2,3 "$TMPD"/c.gta
$GTA create -d 10,10 -c int8,int64 -v 0,3 "$TMPD"/d.gta

$GTA component-extract -k 1,2 < "$TMPD"/b.gta > "$TMPD"/xa.gta
$GTA component-extract -d 0 < "$TMPD"/b.gta > "$TMPD"/xxa.gta
$GTA component-extract -k 0,3 < "$TMPD"/c.gta > "$TMPD"/xd.gta
$GTA component-extract -d 0,3 < "$TMPD"/c.gta > "$TMPD"/xxxa.gta

cmp "$TMPD"/a.gta "$TMPD"/xa.gta
cmp "$TMPD"/a.gta "$TMPD"/xxa.gta
cmp "$TMPD"/a.gta "$TMPD"/xxxa.gta
cmp "$TMPD"/d.gta "$TMPD"/xd.gta

$GTA create -d 10,10 > "$TMPD"/empty0.gta
$GTA create -c uint8,uint16,uint32 -n5 > "$TMPD"/empty1.gta
$GTA create -c uint16 -n5 > "$TMPD"/empty2.gta
$GTA component-extract -d 0,1 "$TMPD"/a.gta > "$TMPD"/xempty0.gta
cmp "$TMPD"/empty0.gta "$TMPD"/xempty0.gta
$GTA component-extract "$TMPD"/empty0.gta > "$TMPD"/xxempty0.gta
cmp "$TMPD"/empty0.gta "$TMPD"/xxempty0.gta
$GTA component-extract -k 1 "$TMPD"/empty1.gta > "$TMPD"/xempty2.gta
cmp "$TMPD"/empty2.gta "$TMPD"/xempty2.gta

rm -r "$TMPD"
