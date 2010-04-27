#!/usr/bin/env bash

# Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 5,5,5 -c uint32 "$TMPD"/a.gta

$GTA uncompress --help 2> "$TMPD"/help.txt

for i in zlib bzip2 xz; do
	$GTA compress --method=$i "$TMPD"/a.gta > "$TMPD"/a-$i.gta
	$GTA uncompress "$TMPD"/a-$i.gta > "$TMPD"/a-$i-u.gta
	cmp "$TMPD"/a.gta "$TMPD"/a-$i-u.gta
done
$GTA uncompress "$TMPD"/a.gta > "$TMPD"/a-u.gta
cmp "$TMPD"/a.gta "$TMPD"/a-u.gta

rm -r "$TMPD"
