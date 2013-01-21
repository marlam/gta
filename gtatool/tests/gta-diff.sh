#!/usr/bin/env bash

# Copyright (C) 2013
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c int8,uint8,int16,uint16,int32,uint32,int64,uint64,float32,float64 -v 42,42,42,42,42,42,42,42,42,42 "$TMPD"/a.gta
$GTA create -d 10,10 -c int8,uint8,int16,uint16,int32,uint32,int64,uint64,float32,float64 -v 24,24,24,24,24,24,24,24,24,24 "$TMPD"/b.gta
$GTA create -d 10,10 -c int8,uint8,int16,uint16,int32,uint32,int64,uint64,float32,float64 -v 18,18,18,18,18,18,18,18,18,18 "$TMPD"/c.gta

$GTA diff "$TMPD"/a.gta "$TMPD"/b.gta > "$TMPD"/d.gta
cmp "$TMPD"/c.gta "$TMPD"/d.gta

$GTA diff -a "$TMPD"/b.gta "$TMPD"/a.gta > "$TMPD"/e.gta
cmp "$TMPD"/c.gta "$TMPD"/e.gta

rm -r "$TMPD"
