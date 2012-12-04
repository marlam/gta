#!/usr/bin/env bash

# Copyright (C) 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10 -c float32,float32,float32 -v 1,2,3 "$TMPD"/a.gta

$GTA to-ply "$TMPD"/a.gta "$TMPD"/b.ply
$GTA to-ply "$TMPD"/c.ply < "$TMPD"/a.gta
cmp "$TMPD"/b.ply "$TMPD"/c.ply

$GTA from-ply "$TMPD"/b.ply "$TMPD"/b.gta
$GTA from-ply "$TMPD"/c.ply > "$TMPD"/c.gta

$GTA tag --unset-all < "$TMPD"/b.gta > "$TMPD"/d.gta
$GTA tag --unset-all < "$TMPD"/c.gta > "$TMPD"/e.gta

cmp "$TMPD"/d.gta "$TMPD"/a.gta
cmp "$TMPD"/e.gta "$TMPD"/a.gta

rm -r "$TMPD"
