#!/usr/bin/env bash

# Copyright (C) 2012, 2013, 2014
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c uint8 -v 42 "$TMPD"/a.gta

$GTA to-csv "$TMPD"/a.gta "$TMPD"/b.csv
$GTA to-csv "$TMPD"/c.csv < "$TMPD"/a.gta
cmp "$TMPD"/b.csv "$TMPD"/c.csv

$GTA from-csv -c uint8 "$TMPD"/b.csv "$TMPD"/b.gta
$GTA from-csv -c uint8 "$TMPD"/c.csv > "$TMPD"/c.gta
cmp "$TMPD"/b.gta "$TMPD"/c.gta
cmp "$TMPD"/b.gta "$TMPD"/a.gta


$GTA create -d 4,3 -c float64 -v 1 >  "$TMPD"/aa.gta
$GTA create -d 3,4 -c float64 -v 2 >> "$TMPD"/aa.gta
echo -e "1,1,1,1\r" >  "$TMPD"/aa.csv
echo -e "1,1,1,1\r" >> "$TMPD"/aa.csv
echo -e "1,1,1,1\r" >> "$TMPD"/aa.csv
echo -e "\r"        >> "$TMPD"/aa.csv
echo -e "2,2,2\r"   >> "$TMPD"/aa.csv
echo -e "2,2,2\r"   >> "$TMPD"/aa.csv
echo -e "2,2,2\r"   >> "$TMPD"/aa.csv
echo -e "2,2,2\r"   >> "$TMPD"/aa.csv

$GTA to-csv   "$TMPD"/aa.gta "$TMPD"/bb.csv
$GTA from-csv "$TMPD"/aa.csv "$TMPD"/bb.gta

cmp "$TMPD"/aa.gta "$TMPD"/bb.gta
cmp "$TMPD"/aa.csv "$TMPD"/bb.csv

rm -r "$TMPD"
