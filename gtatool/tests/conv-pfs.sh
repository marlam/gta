#!/usr/bin/env bash

# Copyright (C) 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 6,8 -c float32,float32 -v 13,17 "$TMPD"/a.gta

$GTA to-pfs "$TMPD"/a.gta "$TMPD"/b.pfs
$GTA to-pfs "$TMPD"/c.pfs < "$TMPD"/a.gta
#cmp "$TMPD"/b.pfs "$TMPD"/c.pfs

$GTA from-pfs "$TMPD"/b.pfs "$TMPD"/b.gta
$GTA from-pfs "$TMPD"/c.pfs > "$TMPD"/c.gta

$GTA tag --unset-all < "$TMPD"/b.gta > "$TMPD"/d.gta
$GTA tag --unset-all < "$TMPD"/c.gta > "$TMPD"/e.gta

cmp "$TMPD"/d.gta "$TMPD"/a.gta
cmp "$TMPD"/e.gta "$TMPD"/a.gta

rm -r "$TMPD"
