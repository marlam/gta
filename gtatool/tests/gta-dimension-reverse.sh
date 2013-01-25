#!/usr/bin/env bash

# Copyright (C) 2010, 2011, 2013
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA dimension-reverse --help 2> "$TMPD"/help.txt

$GTA create -d 5,1 -c uint8 -v 1 "$TMPD"/a1.gta
$GTA create -d 5,2 -c uint8 -v 2 "$TMPD"/a2.gta
$GTA create -d 5,3 -c uint8 -v 3 "$TMPD"/a3.gta
$GTA merge -d 1 "$TMPD"/a1.gta "$TMPD"/a2.gta "$TMPD"/a3.gta > "$TMPD"/a123.gta
$GTA merge -d 1 "$TMPD"/a3.gta "$TMPD"/a2.gta "$TMPD"/a1.gta > "$TMPD"/a321.gta
$GTA compress "$TMPD"/a123.gta > "$TMPD"/za123.gta

$GTA dimension-reverse "$TMPD"/a123.gta > "$TMPD"/b.gta
cmp "$TMPD"/b.gta "$TMPD"/a123.gta

$GTA dimension-reverse -i 0 "$TMPD"/a123.gta > "$TMPD"/c.gta
cmp "$TMPD"/c.gta "$TMPD"/a123.gta

$GTA dimension-reverse -i 1 "$TMPD"/a123.gta > "$TMPD"/d.gta
cmp "$TMPD"/d.gta "$TMPD"/a321.gta

$GTA dimension-reverse -i 1 "$TMPD"/za123.gta > "$TMPD"/e.gta
cmp "$TMPD"/e.gta "$TMPD"/a321.gta

cat "$TMPD"/a123.gta | $GTA dimension-reverse -i 1 > "$TMPD"/f.gta
cmp "$TMPD"/f.gta "$TMPD"/a321.gta

rm -r "$TMPD"
