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

$GTA component-add -c int8 -i 0 -v 0 < "$TMPD"/a.gta > "$TMPD"/xb.gta
$GTA component-add -c int64 -i 3 -v 3 < "$TMPD"/xb.gta > "$TMPD"/xc.gta
$GTA component-add -c int16,int32 -i 1 -v 1,2 < "$TMPD"/d.gta > "$TMPD"/xxc.gta

cmp "$TMPD"/b.gta "$TMPD"/xb.gta
cmp "$TMPD"/c.gta "$TMPD"/xc.gta
cmp "$TMPD"/c.gta "$TMPD"/xxc.gta

rm -r "$TMPD"
