#!/bin/sh
#
# Copyright (C) 2002, 2003 Alexey Voinov <voins@voins.program.ru>
#
testfilename=$1
project=$2
change=$3
searchpath=$4

includedirs=`echo $searchpath | sed -e's/^/-I/;s/:/\/include -I/g;s/$/\/include/'`
libdirs=`echo $searchpath | sed -e's/^/-L/;s/:/\/bin -L/g;s/$/\/bin/'`

mainsource=
for p in `echo $searchpath|tr ':' ' '`; do
	if test -e $p/etc/runtest_hh.cc; then
		mainsource=$p/etc/runtest_hh.cc
	fi
done
if test -z "$mainsource"; then fail "cannot find runtest_hh.cc"; fi

CFLAGS=${CFLAGS:-"-g"}
CFLAGS="$CFLAGS $includedirs"
LDFLAGS="$libdirs -lcppunit $LDFLAGS"
CC=${CC:-g++}
LDFLAGS="$LDFLAGS `grep -e '^LDFLAGS=' $testfilename|sed -e's/^LDFLAGS=//'`"
LIBS="$LIBS `grep -e '^LIBS=' $testfilename|sed -e's/^LIBS=//'`"
CFLAGS="$CFLAGS `grep -e '^CFLAGS=' $testfilename|sed -e's/^CFLAGS=//'`"
export LD_LIBRARY_PATH=$searchpath

echo $CC $CFLAGS $LDFLAGS -include $testfilename -o testprog $mainsource $LIBS
$CC $CFLAGS $LDFLAGS -include $testfilename -o testprog $mainsource $LIBS
if test $? -ne 0; then fail; fi

echo ./testprog
./testprog
if test $? -ne 0; then fail; fi

pass

