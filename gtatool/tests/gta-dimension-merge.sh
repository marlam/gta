#!/usr/bin/env bash

# Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA dimension-merge --help 2> "$TMPD"/help.txt

$GTA create -d 5 -c uint8 -v 42 "$TMPD"/a.gta
$GTA create -d 5 -c uint8 -v 42 "$TMPD"/b.gta
$GTA create -d 5 -c uint8 -v 42 "$TMPD"/c.gta
$GTA create -d 5,3 -c uint8 -v 42 "$TMPD"/d.gta

$GTA dimension-merge "$TMPD"/a.gta "$TMPD"/b.gta "$TMPD"/c.gta > "$TMPD"/xd.gta
cmp "$TMPD"/xd.gta "$TMPD"/d.gta

rm -r "$TMPD"
