#!/bin/sh
ACLOCAL=${ACLOCAL:-aclocal}
AUTOCONF=${AUTOCONF:-autoconf}
AUTOMAKE=${AUTOMAKE:-automake}
AUTOHEADER=${AUTOHEADER:-autoheader}
$ACLOCAL
rm -vf config.cache
$AUTOHEADER
$AUTOCONF
$AUTOMAKE --add-missing
./configure --enable-pam --with-pamdir=/etc/pam.d $@

