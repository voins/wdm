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

CFLAGS=${CFLAGS:-"-g"}
CXXFLAGS=${CXXFLAGS:-""}
CFLAGS="$CFLAGS $includedirs"
LDFLAGS="$libdirs $LDFLAGS"
CXX=${CXX:-g++}
LDFLAGS="$LDFLAGS `grep -e '^LDFLAGS=' $testfilename|sed -e's/^LDFLAGS=//'`"
LIBS="$LIBS `grep -e '^LIBS=' $testfilename|sed -e's/^LIBS=//'`"
CFLAGS="$CFLAGS `grep -e '^CFLAGS=' $testfilename|sed -e's/^CFLAGS=//'`"
export LD_LIBRARY_PATH=$searchpath

echo $CXX $CFLAGS $LDFLAGS $testfilename $LIBS -o testprog
$CXX $CFLAGS $LDFLAGS $testfilename $LIBS -o testprog
if test $? -ne 0; then fail; fi

echo ./testprog
./testprog
if test $? -ne 0; then fail; fi

pass

