#!/usr/bin/env bash

# Copyright (C) 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c uint8 -v 42 "$TMPD"/a.gta

$GTA to-csv "$TMPD"/a.gta "$TMPD"/b.csv
$GTA to-csv "$TMPD"/c.csv < "$TMPD"/a.gta
cmp "$TMPD"/b.csv "$TMPD"/c.csv

$GTA from-csv -c uint8 "$TMPD"/b.csv "$TMPD"/b.gta
$GTA from-csv -c uint8 "$TMPD"/c.csv > "$TMPD"/c.gta
cmp "$TMPD"/b.gta "$TMPD"/c.gta
cmp "$TMPD"/b.gta "$TMPD"/a.gta

rm -r "$TMPD"
