#!/usr/bin/env bash

# Copyright (C) 2012, 2013
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c uint8,uint8,uint8 -v 255,255,255 "$TMPD"/a.gta

$GTA to-magick "$TMPD"/a.gta "$TMPD"/b.ppm
$GTA to-magick "$TMPD"/c.ppm < "$TMPD"/a.gta
cmp "$TMPD"/b.ppm "$TMPD"/c.ppm

$GTA from-magick "$TMPD"/b.ppm "$TMPD"/b.gta
$GTA from-magick "$TMPD"/c.ppm > "$TMPD"/c.gta

$GTA tag --unset-all < "$TMPD"/b.gta > "$TMPD"/d.gta
$GTA tag --unset-all < "$TMPD"/c.gta > "$TMPD"/e.gta

cmp "$TMPD"/d.gta "$TMPD"/a.gta
cmp "$TMPD"/e.gta "$TMPD"/a.gta

rm -r "$TMPD"
