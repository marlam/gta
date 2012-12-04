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

$GTA to-datraw "$TMPD"/a.gta "$TMPD"/b.dat
$GTA to-datraw "$TMPD"/c.dat < "$TMPD"/a.gta
#cmp "$TMPD"/b.dat "$TMPD"/c.dat
cmp "$TMPD"/b.raw "$TMPD"/c.raw

$GTA from-datraw "$TMPD"/b.dat "$TMPD"/b.gta
$GTA from-datraw "$TMPD"/c.dat > "$TMPD"/c.gta
cmp "$TMPD"/b.gta "$TMPD"/c.gta
cmp "$TMPD"/b.gta "$TMPD"/a.gta

rm -r "$TMPD"
