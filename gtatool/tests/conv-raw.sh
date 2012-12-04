#!/usr/bin/env bash

# Copyright (C) 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 6,8 -c uint8 -v 42 "$TMPD"/a.gta

$GTA to-raw "$TMPD"/a.gta "$TMPD"/b.raw
$GTA to-raw "$TMPD"/c.raw < "$TMPD"/a.gta
cmp "$TMPD"/b.raw "$TMPD"/c.raw

$GTA from-raw -d 6,8 -c uint8 "$TMPD"/b.raw "$TMPD"/b.gta
$GTA from-raw -d 6,8 -c uint8 "$TMPD"/c.raw > "$TMPD"/c.gta

$GTA tag --unset-all < "$TMPD"/b.gta > "$TMPD"/d.gta
$GTA tag --unset-all < "$TMPD"/c.gta > "$TMPD"/e.gta

cmp "$TMPD"/d.gta "$TMPD"/a.gta
cmp "$TMPD"/e.gta "$TMPD"/a.gta

rm -r "$TMPD"
