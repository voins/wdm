#!/bin/sh
need="$1"
target="$2"

sed $need \
	-e 's@\[prepost.*\[.*\]\]@-I./include@' \
	-e 's@\[\([^\[]*\)\]@\$(\1)@g' \
	-e 's@;$@@' \
	-e 's@/\*\*\(.*\)\*/@\1@' \
	-e 's@/\*\(.*\)\*/@# \1@' \
	> $target


