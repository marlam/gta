#!/usr/bin/env bash

# Copyright (C) 2011, 2013
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c int8,int16,int32,int64 -v 0,1,2,3 "$TMPD"/a.gta
$GTA create -d 10,10 -c int64,int32,int16,int8 -v 3,2,1,0 "$TMPD"/b.gta
$GTA create -d 10,10 -c int32,int8,int16,int64 -v 2,0,1,3 "$TMPD"/c.gta

$GTA component-reorder < "$TMPD"/a.gta > "$TMPD"/xa.gta
$GTA component-reorder -i 3,2,1,0 < "$TMPD"/a.gta > "$TMPD"/xb.gta
$GTA component-reorder -i 2,0,1,3 < "$TMPD"/a.gta > "$TMPD"/xc.gta

cmp "$TMPD"/a.gta "$TMPD"/xa.gta
cmp "$TMPD"/b.gta "$TMPD"/xb.gta
cmp "$TMPD"/c.gta "$TMPD"/xc.gta

$GTA create -d 10 -n5 > "$TMPD"/empty0.gta
$GTA create -c uint8,uint16 -n5 > "$TMPD"/empty1.gta
$GTA create -c uint16,uint8 -n5 > "$TMPD"/empty2.gta
$GTA component-reorder "$TMPD"/empty0.gta > "$TMPD"/xempty0.gta
cmp "$TMPD"/empty0.gta "$TMPD"/xempty0.gta
$GTA component-reorder -i 1,0 "$TMPD"/empty1.gta > "$TMPD"/xempty2.gta
cmp "$TMPD"/empty2.gta "$TMPD"/xempty2.gta

rm -r "$TMPD"
