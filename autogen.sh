#!/bin/sh
ACLOCAL=${ACLOCAL:-aclocal}
AUTOCONF=${AUTOCONF:-autoconf}
AUTOMAKE=${AUTOMAKE:-automake}
$ACLOCAL
rm -vf config.cache
$AUTOCONF
$AUTOMAKE --add-missing
./configure --enable-pam --with-pamdir=/etc/pam.d $@

