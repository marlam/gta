#!/usr/bin/env bash

# Copyright (C) 2011, 2013
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c uint8 -v 42 "$TMPD"/a.gta
$GTA create -d 10,10 -c uint8 -v 117 "$TMPD"/b.gta

$GTA component-set -i 0 -v 117 "$TMPD"/a.gta > "$TMPD"/c.gta
cmp "$TMPD"/b.gta "$TMPD"/c.gta

$GTA create -d 10 -n5 > "$TMPD"/empty0.gta
$GTA create -c uint8 -n5 > "$TMPD"/empty1.gta
$GTA component-set "$TMPD"/empty0.gta > "$TMPD"/xempty0.gta
cmp "$TMPD"/empty0.gta "$TMPD"/xempty0.gta
$GTA component-set -i 0 -v 117 "$TMPD"/empty1.gta > "$TMPD"/xempty1.gta
cmp "$TMPD"/empty1.gta "$TMPD"/xempty1.gta

rm -r "$TMPD"
