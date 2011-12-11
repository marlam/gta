#!/usr/bin/env bash

# Copyright (C) 2011
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

rm -r "$TMPD"
