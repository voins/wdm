# automake input for wdm documentation files

configfilesdir = $(DMDIR)

man_MANS = wdm.1 wdmLogin.1

CLEANFILES = wdm.1 wdmLogin.1

EXTRA_DIST = wdm.man.in wdmLogin.man

wdm.1: wdm.man.in Makefile
	-rm -f wdm.1
	sed -e "s:#configdir#:$(configfilesdir):" \
		$(srcdir)/wdm.man.in > wdm.1
	chmod 644 wdm.1

wdmLogin.1: wdmLogin.man Makefile
	-rm -f wdmLogin.1
	cp -p wdmLogin.man wdmLogin.1
	chmod 644 wdmLogin.1

