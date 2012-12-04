#!/usr/bin/env bash

# Copyright (C) 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA from-ffmpeg --help 2> "$TMPD"/gta-from-ffmpeg-help.txt

# Create a 8x8 PGM image file manually
echo "P6" >> "$TMPD"/a.pnm
echo "8 8" >> "$TMPD"/a.pnm
echo "255" >> "$TMPD"/a.pnm
echo -n '****************************************************************' >> "$TMPD"/a.pnm
echo -n '****************************************************************' >> "$TMPD"/a.pnm
echo -n '****************************************************************' >> "$TMPD"/a.pnm
# This corresponds to the following GTA:
$GTA create -d 8,8 -c uint8,uint8,uint8 -v 42,42,42 "$TMPD"/a.gta

$GTA from-ffmpeg "$TMPD"/a.pnm "$TMPD"/b.gta
$GTA from-ffmpeg "$TMPD"/a.pnm > "$TMPD"/c.gta
$GTA from-ffmpeg -s 1 "$TMPD"/a.pnm "$TMPD"/d.gta

$GTA tag --unset-all < "$TMPD"/b.gta > "$TMPD"/e.gta
$GTA tag --unset-all < "$TMPD"/c.gta > "$TMPD"/f.gta
$GTA tag --unset-all < "$TMPD"/d.gta > "$TMPD"/g.gta

cmp "$TMPD"/e.gta "$TMPD"/a.gta
cmp "$TMPD"/f.gta "$TMPD"/a.gta
cmp "$TMPD"/g.gta "$TMPD"/a.gta

rm -r "$TMPD"
