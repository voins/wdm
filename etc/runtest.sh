#!/bin/sh
#
# Copyright (C) 2002, 2003 Alexey Voinov <voins@voins.program.ru>
#
testfilename="$1"
export project="$2"
export change="$3"
export searchpath="$4"

export here=`pwd`
if test $? -ne 0; then exit 2; fi

searchandrun()
{
	prog=$1
	shift
	result=
	for p in `echo $searchpath|tr ':' ' '`; do
		if test -e $p/$prog; then
			result=$p/$prog
			break;
		fi
	done
	if test -z "$result"; then fail "cannot find script $prog"; fi
	echo source $result $@
	source $result $@
}

pass()
{
	set +x
	echo PASSED [$testfilename]
	cd $here
	rm -rf $work
	exit 0
}

fail()
{
	set +x
	cd $here
	rm -rf $work
	echo FAILED [$testfilename${@:+": $@"}]
	exit 1
}

no_result()
{
	set +x
	cd $here
	rm -rf $work
	echo NO RESULT [$testfilename${@:+": $@"}]
}
trap "no_result" 1 2 3 15

work=`mktemp -dt test.XXXXXXXXXX`
if test $? -ne 0; then no_result "cannot create temporary directory"; fi
cd $work
if test $? -ne 0; then no_result "cannot cd to temporary directory"; fi

export PAGER=cat

if echo $testfilename | grep -q -e '\.cc$'; then
	searchandrun etc/runtest_cc.sh $@
elif echo $testfilename | grep -q -e '\.hh$'; then
	searchandrun etc/runtest_hh.sh $@
elif echo $testfilename | grep -q -e '\.sh$'; then
	searchandrun etc/runtest_sh.sh $@
elif echo $testfilename | grep -q -e '\.c$'; then
	searchandrun etc/runtest_c.sh $@
else
	echo "don't know how to run this test [$testfilename]"
	exit 1
fi

fail "something wrong, shouldn\'t get there."

