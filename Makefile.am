## Process this file with automake to produce Makefile.in for DisplayMaker

SUBDIRS = \
	doc \
	src

EXTRA_DIST = \
	autogen.sh \
	README.pam \
	wdm.spec.in \
	wdm.spec \
	xdm-332.tar.gz

dist-hook:
	mkdir $(distdir)/patches
	cp -p patches/* $(distdir)/patches
	cp -p patches/* $(distdir)/patches
	-rm -f wdm.spec
	sed -e "s:#VERSION#:@VERSION@:" \
		$(srcdir)/wdm.spec.in >wdm.spec
	cp -p wdm.spec $(distdir)/wdm-@VERSION@.spec
