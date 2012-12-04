#!/usr/bin/env bash

# Copyright (C) 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 100 -c int16 -v 42 "$TMPD"/a.gta

$GTA to-sndfile "$TMPD"/a.gta "$TMPD"/b.wav
$GTA to-sndfile "$TMPD"/c.wav < "$TMPD"/a.gta
cmp "$TMPD"/b.wav "$TMPD"/c.wav

$GTA from-sndfile "$TMPD"/b.wav "$TMPD"/b.gta
$GTA from-sndfile "$TMPD"/c.wav > "$TMPD"/c.gta

$GTA tag --unset-all < "$TMPD"/b.gta > "$TMPD"/d.gta
$GTA tag --unset-all < "$TMPD"/c.gta > "$TMPD"/e.gta

cmp "$TMPD"/d.gta "$TMPD"/a.gta
cmp "$TMPD"/e.gta "$TMPD"/a.gta

rm -r "$TMPD"
