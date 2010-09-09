#!/usr/bin/env bash

# Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA dimension-add --help 2> "$TMPD"/help.txt

$GTA create -d 3 -c uint8 "$TMPD"/a.gta
$GTA create -d 3,1 -c uint8 "$TMPD"/b.gta
$GTA create -d 1,3 -c uint8 "$TMPD"/c.gta

$GTA dimension-add -d 1 < "$TMPD"/a.gta > "$TMPD"/xb.gta
cmp "$TMPD"/xb.gta "$TMPD"/b.gta

$GTA dimension-add -d 0 < "$TMPD"/a.gta > "$TMPD"/xc.gta
cmp "$TMPD"/xc.gta "$TMPD"/c.gta

rm -r "$TMPD"
