#!/bin/sh
#
# Copyright (C) 2002, 2003 Alexey Voinov <voins@voins.program.ru>
#
testfilename=$1
project=$2
change=$3
searchpath=$4

echo $SHELL $testfilename $project $change $searchpath
$SHELL $testfilename $project $change $searchpath
if test $? -ne 0; then fail; fi

pass
