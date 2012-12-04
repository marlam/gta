#!/usr/bin/env bash

# Copyright (C) 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

set >> "$TMPD"/env

$GTA create -d 10,10 -c uint8 -v 42 "$TMPD"/a.gta
$GTA create -d 10,10 -c uint8 -v 117 "$TMPD"/b.gta

$GTA component-compute -e 'c0=42' "$TMPD"/b.gta > "$TMPD"/c.gta
cmp "$TMPD"/a.gta "$TMPD"/c.gta

rm -r "$TMPD"
