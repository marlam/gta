#!/usr/bin/env bash

# Copyright (C) 2010, 2011
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c uint8 -v 42 "$TMPD"/a.gta
$GTA create -d 10,10 -c uint8 -v 42 "$TMPD"/b.gta
$GTA create -d 10,10 -c uint8 -v 42 "$TMPD"/c.gta
$GTA create -d 10,30 -c uint8 -v 42 "$TMPD"/abc.gta

$GTA merge -d 1 "$TMPD"/a.gta "$TMPD"/b.gta "$TMPD"/c.gta > "$TMPD"/d.gta
cmp "$TMPD"/abc.gta "$TMPD"/d.gta

rm -r "$TMPD"
