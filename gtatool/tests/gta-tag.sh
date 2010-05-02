#!/usr/bin/env bash

# Copyright (C) 2010  Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

set -e

TMPD="`mktemp -d tmp-\`basename $0 .sh\`.XXXXXX`"

$GTA create -d 7,11,13 -c int8,int8,int8 -n 2 "$TMPD"/a.gta

$GTA tag --help 2> "$TMPD"/help.txt

PING="$TMPD"/ping.gta
PONG="$TMPD"/pong.gta
$GTA tag "$TMPD"/a.gta > "$PING"

$GTA tag --set-global=globalfoo=bar "$PING" > "$PONG"
PTMP="$PING"; PING="$PONG"; PONG="$PTMP"
$GTA tag --set-dimension=0,dim0foo=bar "$PING" > "$PONG"
PTMP="$PING"; PING="$PONG"; PONG="$PTMP"
$GTA tag --set-dimension=1,dim1foo=bar "$PING" > "$PONG"
PTMP="$PING"; PING="$PONG"; PONG="$PTMP"
$GTA tag --set-dimension=2,dim2foo=bar "$PING" > "$PONG"
PTMP="$PING"; PING="$PONG"; PONG="$PTMP"
$GTA tag --set-component=0,cmp0foo=bar "$PING" > "$PONG"
PTMP="$PING"; PING="$PONG"; PONG="$PTMP"
$GTA tag --set-component=1,cmp1foo=bar "$PING" > "$PONG"
PTMP="$PING"; PING="$PONG"; PONG="$PTMP"
$GTA tag --set-component=2,cmp2foo=bar "$PING" > "$PONG"
PTMP="$PING"; PING="$PONG"; PONG="$PTMP"

$GTA tag --get-global=baz "$PING" > /dev/null 2> "$TMPD"/out.txt
$GTA tag --get-global=foo "$PING" > /dev/null 2> "$TMPD"/out.txt

$GTA tag --unset-global=baz \
	--unset-global-all \
	--unset-dimension=0,dim0foo \
	--unset-dimension-all=0 \
	--unset-dimension-all=1 \
	--unset-dimension-all=2 \
	--unset-component-all=0 \
	--unset-component-all=1 \
	--unset-component-all=2 \
	--unset-all "$PING" > "$PONG"
cmp "$TMPD"/a.gta "$PONG"

rm -r "$TMPD"