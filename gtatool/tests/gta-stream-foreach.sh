#!/usr/bin/env bash

# Copyright (C) 2013
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA stream-foreach --help 2> "$TMPD"/help.txt

touch "$TMPD"/0.gta
$GTA create -d 10,10 -c uint8 -n 1 "$TMPD"/1.gta
$GTA create          -c uint8 -n 5 "$TMPD"/5.gta
$GTA create -d 10,10          -n 9 "$TMPD"/9.gta

for i in 0 1 5 9; do
    $GTA stream-foreach "$GTA uncompress" "$TMPD"/$i.gta > "$TMPD"/x$i.gta
    cmp "$TMPD"/$i.gta "$TMPD"/x$i.gta
    $GTA stream-foreach "$GTA uncompress" < "$TMPD"/$i.gta > "$TMPD"/y$i.gta
    cmp "$TMPD"/$i.gta "$TMPD"/y$i.gta
done

rm -r "$TMPD"
