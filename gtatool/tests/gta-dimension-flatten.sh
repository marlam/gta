#!/usr/bin/env bash

# Copyright (C) 2010, 2011
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA dimension-flatten --help 2> "$TMPD"/help.txt

$GTA create -d 3,5 -c uint8 -v 42 "$TMPD"/a.gta
$GTA create -d 15 -c uint8 -v 42 "$TMPD"/b.gta

$GTA dimension-flatten < "$TMPD"/a.gta > "$TMPD"/xb.gta
cmp "$TMPD"/xb.gta "$TMPD"/b.gta

rm -r "$TMPD"
