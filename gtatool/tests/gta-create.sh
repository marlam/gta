#!/usr/bin/env bash

# Copyright (C) 2010, 2011, 2013
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create --help 2> "$TMPD"/help.txt

$GTA create -d 10,10,10 \
	-c blob256,int8,uint8,int16,uint16,int32,uint32,int64,uint64,int128,uint128,float32,float64,float128,cfloat32,cfloat64,cfloat128 \
	-n 2 "$TMPD"/a.gta

$GTA create -d 10 \
	-c  blob256,int8,uint8,int16,uint16,int32,uint32,int64,uint64,float32,float64,cfloat32,cfloat64 \
	-v "      0,   1,    2,    3,     4,    5,     6,    7,     8,    0.1,    0.2, 0.3,0.4, 0.5,0.6" \
	> "$TMPD"/b.gta

$GTA create -d 10 -n5 > "$TMPD"/empty0.gta
$GTA create -c uint8 -n5 > "$TMPD"/empty1.gta

rm -r "$TMPD"
