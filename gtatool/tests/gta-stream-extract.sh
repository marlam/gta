#!/usr/bin/env bash

# Copyright (C) 2010, 2011
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c uint8 -v 0 "$TMPD"/0.gta
$GTA create -d 10,10 -c uint8 -v 1 "$TMPD"/1.gta
$GTA create -d 10,10 -c uint8 -v 2 "$TMPD"/2.gta
$GTA stream-merge "$TMPD"/0.gta "$TMPD"/1.gta "$TMPD"/2.gta > "$TMPD"/012.gta

$GTA stream-extract --help 2> "$TMPD"/help.txt

$GTA stream-extract -- -0 "$TMPD"/012.gta > "$TMPD"/x0.gta
cmp "$TMPD"/x0.gta "$TMPD"/0.gta

$GTA stream-extract 1-1 "$TMPD"/012.gta > "$TMPD"/x1.gta
cmp "$TMPD"/x1.gta "$TMPD"/1.gta

$GTA stream-extract 2- "$TMPD"/012.gta > "$TMPD"/x2.gta
cmp "$TMPD"/x2.gta "$TMPD"/2.gta

$GTA stream-extract 0,1-1,-2 "$TMPD"/012.gta > "$TMPD"/x012.gta
cmp "$TMPD"/x012.gta "$TMPD"/012.gta

rm -r "$TMPD"
