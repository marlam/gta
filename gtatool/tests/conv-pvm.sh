#!/usr/bin/env bash

# Copyright (C) 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10,10 -c uint8 -v 42 "$TMPD"/a.gta

$GTA to-pvm "$TMPD"/a.gta "$TMPD"/b.pvm
$GTA to-pvm "$TMPD"/c.pvm < "$TMPD"/a.gta
cmp "$TMPD"/b.pvm "$TMPD"/c.pvm

$GTA from-pvm "$TMPD"/b.pvm "$TMPD"/b.gta
$GTA from-pvm "$TMPD"/c.pvm > "$TMPD"/c.gta

$GTA tag --unset-all < "$TMPD"/b.gta > "$TMPD"/d.gta
$GTA tag --unset-all < "$TMPD"/c.gta > "$TMPD"/e.gta

cmp "$TMPD"/d.gta "$TMPD"/a.gta
cmp "$TMPD"/e.gta "$TMPD"/a.gta

rm -r "$TMPD"
