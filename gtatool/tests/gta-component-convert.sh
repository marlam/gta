#!/usr/bin/env bash

# Copyright (C) 2011, 2013
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c int8,int16,cfloat32 -v 42,42,42,0 "$TMPD"/a.gta
$GTA create -d 10,10 -c float32,float32,float32 -v 42,42,42 "$TMPD"/b.gta

$GTA component-convert -c float32,float32,float32 < "$TMPD"/a.gta > "$TMPD"/xb.gta
cmp "$TMPD"/b.gta "$TMPD"/xb.gta

$GTA create -c uint8 -n5 > "$TMPD"/empty0.gta
$GTA component-convert -c int16 "$TMPD"/empty0.gta > "$TMPD"/t.gta
$GTA component-convert -c uint8 "$TMPD"/t.gta > "$TMPD"/xempty0.gta
cmp "$TMPD"/empty0.gta "$TMPD"/xempty0.gta

rm -r "$TMPD"
