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

$GTA to-rat "$TMPD"/a.gta "$TMPD"/b.rat
$GTA to-rat "$TMPD"/c.rat < "$TMPD"/a.gta
cmp "$TMPD"/b.rat "$TMPD"/c.rat

$GTA from-rat "$TMPD"/b.rat "$TMPD"/b.gta
$GTA from-rat "$TMPD"/c.rat > "$TMPD"/c.gta

$GTA tag --unset-all < "$TMPD"/b.gta > "$TMPD"/d.gta
$GTA tag --unset-all < "$TMPD"/c.gta > "$TMPD"/e.gta

cmp "$TMPD"/d.gta "$TMPD"/a.gta
cmp "$TMPD"/e.gta "$TMPD"/a.gta

rm -r "$TMPD"
