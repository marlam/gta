#!/usr/bin/env bash

# Copyright (C) 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 2,4,6 -c uint64 -v 42 "$TMPD"/a.gta

$GTA to-teem "$TMPD"/a.gta "$TMPD"/b.nrrd
$GTA to-teem "$TMPD"/c.nrrd < "$TMPD"/a.gta
cmp "$TMPD"/b.nrrd "$TMPD"/c.nrrd

$GTA from-teem "$TMPD"/b.nrrd "$TMPD"/b.gta
$GTA from-teem "$TMPD"/c.nrrd > "$TMPD"/c.gta

$GTA tag --unset-all < "$TMPD"/b.gta > "$TMPD"/d.gta
$GTA tag --unset-all < "$TMPD"/c.gta > "$TMPD"/e.gta

cmp "$TMPD"/d.gta "$TMPD"/a.gta
cmp "$TMPD"/e.gta "$TMPD"/a.gta

rm -r "$TMPD"
