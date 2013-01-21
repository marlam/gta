#!/usr/bin/env bash

# Copyright (C) 2013
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 10,10 -c int8,uint8,int16,uint16,int32,uint32,int64,uint64 -v 2,2,2,2,2,2,2,2 "$TMPD"/2.gta
$GTA create -d 10,10 -c int8,uint8,int16,uint16,int32,uint32,int64,uint64 -v 4,4,4,4,4,4,4,4 "$TMPD"/4.gta
$GTA create -d 10,10 -c int8,uint8,int16,uint16,int32,uint32,int64,uint64 -v 6,6,6,6,6,6,6,6 "$TMPD"/6.gta
$GTA create -d 10,10 -c int8,uint8,int16,uint16,int32,uint32,int64,uint64 -v 8,8,8,8,8,8,8,8 "$TMPD"/8.gta

$GTA layer -m min "$TMPD"/2.gta "$TMPD"/4.gta > "$TMPD"/a.gta
cmp "$TMPD"/2.gta "$TMPD"/a.gta

$GTA layer -m max "$TMPD"/2.gta "$TMPD"/4.gta > "$TMPD"/b.gta
cmp "$TMPD"/4.gta "$TMPD"/b.gta

$GTA layer -m add "$TMPD"/2.gta "$TMPD"/4.gta > "$TMPD"/c.gta
cmp "$TMPD"/6.gta "$TMPD"/c.gta

$GTA layer -m sub "$TMPD"/6.gta "$TMPD"/4.gta > "$TMPD"/d.gta
cmp "$TMPD"/2.gta "$TMPD"/d.gta

$GTA layer -m mul "$TMPD"/2.gta "$TMPD"/4.gta > "$TMPD"/e.gta
cmp "$TMPD"/8.gta "$TMPD"/e.gta

$GTA layer -m div "$TMPD"/8.gta "$TMPD"/4.gta > "$TMPD"/f.gta
cmp "$TMPD"/2.gta "$TMPD"/f.gta

$GTA layer -m and "$TMPD"/6.gta "$TMPD"/4.gta > "$TMPD"/g.gta
cmp "$TMPD"/4.gta "$TMPD"/g.gta

$GTA layer -m or  "$TMPD"/2.gta "$TMPD"/4.gta > "$TMPD"/h.gta
cmp "$TMPD"/6.gta "$TMPD"/h.gta

$GTA layer -m xor "$TMPD"/6.gta "$TMPD"/4.gta > "$TMPD"/i.gta
cmp "$TMPD"/2.gta "$TMPD"/i.gta

$GTA create -d 10,10 -c float32,float64 -v 2,2 "$TMPD"/f2.gta
$GTA create -d 10,10 -c float32,float64 -v 4,4 "$TMPD"/f4.gta
$GTA create -d 10,10 -c float32,float64 -v 6,6 "$TMPD"/f6.gta
$GTA create -d 10,10 -c float32,float64 -v 8,8 "$TMPD"/f8.gta

$GTA layer -m min "$TMPD"/f2.gta "$TMPD"/f4.gta > "$TMPD"/j.gta
cmp "$TMPD"/f2.gta "$TMPD"/j.gta

$GTA layer -m max "$TMPD"/f2.gta "$TMPD"/f4.gta > "$TMPD"/k.gta
cmp "$TMPD"/f4.gta "$TMPD"/k.gta

$GTA layer -m add "$TMPD"/f2.gta "$TMPD"/f4.gta > "$TMPD"/l.gta
cmp "$TMPD"/f6.gta "$TMPD"/l.gta

$GTA layer -m sub "$TMPD"/f6.gta "$TMPD"/f4.gta > "$TMPD"/m.gta
cmp "$TMPD"/f2.gta "$TMPD"/m.gta

$GTA layer -m mul "$TMPD"/f2.gta "$TMPD"/f4.gta > "$TMPD"/n.gta
cmp "$TMPD"/f8.gta "$TMPD"/n.gta

$GTA layer -m div "$TMPD"/f8.gta "$TMPD"/f4.gta > "$TMPD"/o.gta
cmp "$TMPD"/f2.gta "$TMPD"/o.gta

rm -r "$TMPD"
