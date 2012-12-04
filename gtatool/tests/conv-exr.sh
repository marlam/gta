#!/usr/bin/env bash

# Copyright (C) 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c float32 -v 42 "$TMPD"/a.gta

$GTA to-exr "$TMPD"/a.gta "$TMPD"/b.exr
$GTA to-exr "$TMPD"/c.exr < "$TMPD"/a.gta
cmp "$TMPD"/b.exr "$TMPD"/c.exr

$GTA from-exr "$TMPD"/b.exr "$TMPD"/b.gta
$GTA from-exr "$TMPD"/c.exr > "$TMPD"/c.gta
cmp "$TMPD"/b.gta "$TMPD"/c.gta

$GTA tag --unset-all < "$TMPD"/b.gta > "$TMPD"/d.gta
cmp "$TMPD"/d.gta "$TMPD"/a.gta

rm -r "$TMPD"
