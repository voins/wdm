#!/bin/sh
aclocal
rm -vf config.cache
autoconf
automake --add-missing
./configure --enable-pam --with-pamdir=/etc/pam.d $@
