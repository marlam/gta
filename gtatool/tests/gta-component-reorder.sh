#!/usr/bin/env bash

# Copyright (C) 2011
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

rm -r "$TMPD"
