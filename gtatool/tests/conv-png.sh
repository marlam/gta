#!/usr/bin/env bash

# Copyright (C) 2014
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c uint8 -v 42 "$TMPD"/a8.gta

$GTA to-png "$TMPD"/a8.gta "$TMPD"/b8.png
$GTA to-png "$TMPD"/c8.png < "$TMPD"/a8.gta
cmp "$TMPD"/b8.png "$TMPD"/c8.png

$GTA from-png "$TMPD"/b8.png "$TMPD"/b8.gta
$GTA from-png "$TMPD"/c8.png > "$TMPD"/c8.gta

$GTA tag --unset-all < "$TMPD"/b8.gta > "$TMPD"/d8.gta
$GTA tag --unset-all < "$TMPD"/c8.gta > "$TMPD"/e8.gta

cmp "$TMPD"/d8.gta "$TMPD"/a8.gta
cmp "$TMPD"/e8.gta "$TMPD"/a8.gta

$GTA create -d 10,10 -c uint8 -v 42 "$TMPD"/a16.gta

$GTA to-png "$TMPD"/a16.gta "$TMPD"/b16.png
$GTA to-png "$TMPD"/c16.png < "$TMPD"/a16.gta
cmp "$TMPD"/b16.png "$TMPD"/c16.png

$GTA from-png "$TMPD"/b16.png "$TMPD"/b16.gta
$GTA from-png "$TMPD"/c16.png > "$TMPD"/c16.gta

$GTA tag --unset-all < "$TMPD"/b16.gta > "$TMPD"/d16.gta
$GTA tag --unset-all < "$TMPD"/c16.gta > "$TMPD"/e16.gta

cmp "$TMPD"/d16.gta "$TMPD"/a16.gta
cmp "$TMPD"/e16.gta "$TMPD"/a16.gta

rm -r "$TMPD"
