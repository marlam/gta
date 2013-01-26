#!/usr/bin/env bash

# Copyright (C) 2010, 2011, 2013
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 5,5,5 -c uint32 "$TMPD"/a.gta

$GTA uncompress --help 2> "$TMPD"/help.txt

for i in zlib bzip2 xz zlib1 zlib2 zlib3 zlib4 zlib5 zlib6 zlib7 zlib8 zlib9; do
	$GTA compress --method=$i "$TMPD"/a.gta > "$TMPD"/a-$i.gta
	$GTA uncompress "$TMPD"/a-$i.gta > "$TMPD"/a-$i-u.gta
	cmp "$TMPD"/a.gta "$TMPD"/a-$i-u.gta
done
$GTA uncompress "$TMPD"/a.gta > "$TMPD"/a-u.gta
cmp "$TMPD"/a.gta "$TMPD"/a-u.gta

$GTA create -d 10 > "$TMPD"/empty0.gta
$GTA create -c uint8 > "$TMPD"/empty1.gta
$GTA compress "$TMPD"/empty0.gta > "$TMPD"/zempty0.gta
$GTA uncompress "$TMPD"/zempty0.gta > "$TMPD"/xempty0.gta
cmp "$TMPD"/empty0.gta "$TMPD"/xempty0.gta
$GTA compress "$TMPD"/empty1.gta > "$TMPD"/zempty1.gta
$GTA uncompress "$TMPD"/zempty1.gta > "$TMPD"/xempty1.gta
cmp "$TMPD"/empty1.gta "$TMPD"/xempty1.gta

rm -r "$TMPD"
