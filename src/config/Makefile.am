# automake input for wdm configuration files

configfilesdir = $(DMDIR)

bindir = $(DMDIR)

BINDIR = $(exec_prefix)/bin

bin_SCRIPTS = \
		GiveConsole \
		TakeConsole \
		Xsession \
		Xsession.XFree86 \
		Xsetup_0 \
		Xclients \
		wdmReconfig

configfiles_DATA = \
		Xaccess \
		Xresources \
		Xservers \
		Xservers.ws \
		Xservers.fs \
		wdm-config

CLEANFILES = wdm-config Xsetup_0 Xservers Xservers.ws Xsession Xclients

wdm-config: wdm-config.in Makefile
	-rm -f wdm-config
	sed -e "s:#configdir#:$(configfilesdir):" \
		-e "s:#bindir#:$(BINDIR):" \
		-e "s:#SHUTDOWN#:$(SHUTDOWN):" \
		-e "s:#logdir#:$(LOGDIR):" \
		-e "s:#rundir#:$(RUNDIR):" \
		-e "s:#pixmapfile#:$(PIXMAPFILE):" \
		-e "s:#pixmapcomment#:$(PIXMAPCOMMENT):" \
		-e "s/#wmlist#/$(WMLIST)/" \
		$(srcdir)/wdm-config.in > wdm-config
	chmod 644 wdm-config 

Xsetup_0: Xsetup_0.in Makefile
	-rm -f Xsetup_0
	sed -e "s:#xconsole#:$(XCONSOLE):" \
	    -e "s:#SHELL#:$(SHELL_SH):" \
		$(srcdir)/Xsetup_0.in > Xsetup_0
	chmod 755 Xsetup_0

Xsession: Xsession.in Makefile
	-rm -f Xsession
	sed -e "s:#configdir#:$(configfilesdir):" \
	    -e "s:#SHELL#:$(SHELL_LOGIN):" \
	    -e "s:#XRDB#:$(XRDB_PATH):" \
	    -e "s:#FAILSAFE#:$(FAILSAFE_PATH):" \
		$(srcdir)/Xsession.in > Xsession
	chmod 755 Xsession

Xclients: Xclients.in Makefile
	-rm -f Xclients
	sed -e "s:#XRDB#:$(XRDB_PATH):" \
	    -e "s:#SHELL#:$(SHELL_LOGIN):" \
	    -e "s'#WMAKER_PATH#'$(WMAKER_PATH)'" \
	    -e "s'#wmakercm#'$(wmakercm)'" \
	    -e "s'#AFTERSTEP_PATH#'$(AFTERSTEP_PATH)'" \
	    -e "s'#afterstepcm#'$(afterstepcm)'" \
	    -e "s'#BLACKBOX_PATH#'$(BLACKBOX_PATH)'" \
	    -e "s'#blackboxcm#'$(blackboxcm)'" \
	    -e "s'#ICEWM_PATH#'$(ICEWM_PATH)'" \
	    -e "s'#icewmcm#'$(icewmcm)'" \
	    -e "s'#XSESSION_PATH#'$(XSESSION_PATH)'" \
	    -e "s'#xsessioncm#'$(xsessioncm)'" \
	    -e "s'#STARTKDE_PATH#'$(STARTKDE_PATH)'" \
	    -e "s'#startkdecm#'$(STARTKDECM)'" \
	    -e "s'#winmgrname#'$(WINMGR_NAME)'" \
	    -e "s'#WINMGR_PATH#'$(WINMGR_PATH)'" \
	    -e "s'#winmgrcm#'$(WINMGRCM)'" \
		$(srcdir)/Xclients.in > Xclients
	chmod 755 Xclients

wdmReconfig: wdmReconfig.in Makefile
	-rm -f wdmReconfig
	sed -e "s:#SHELL#:$(SHELL_SH):" \
		-e "s:@configdir@:$(configfilesdir):" \
		-e "s:@BINDIR@:$(BINDIR):" \
		-e "s:@shutdown@:$(SHUTDOWN):" \
		-e "s:@logdir@:$(LOGDIR):" \
		-e "s:@rundir@:$(RUNDIR):" \
		-e "s:@pixmapfile@:$(PIXMAPFILE):" \
		-e "s:@pixmapcomment@:$(PIXMAPCOMMENT):" \
		-e "s:@XRDB@:$(XRDB_PATH):" \
		-e "s:@shell_login@:$(SHELL_LOGIN):" \
		$(srcdir)/wdmReconfig.in > wdmReconfig
	chmod 755 wdmReconfig

Xservers: Xservers.in Makefile
	-rm -f Xservers
	sed -e "s:#X#:$(DEF_SERVER):" \
		$(srcdir)/Xservers.in > Xservers
	chmod 644 Xservers

Xservers.ws: Xservers.ws.in Makefile
	-rm -f Xservers.ws
	sed -e "s:#X#:$(DEF_SERVER):" \
		$(srcdir)/Xservers.ws.in > Xservers.ws
	chmod 644 Xservers.ws

EXTRA_DIST = \
	wdm.pamd \
	$(configfiles_DATA) \
	$(bin_SCRIPTS)

install-data-hook:
	$(INSTALL) -d $(DESTDIR)$(configfilesdir)/authdir
	chmod 700 $(DESTDIR)$(configfilesdir)/authdir
	$(INSTALL) -m 644 $(srcdir)/wdm-config.in $(DESTDIR)$(configfilesdir)
	$(INSTALL) -m 600 $(srcdir)/wdm-config $(DESTDIR)$(configfilesdir)
	$(INSTALL) -m 644 $(srcdir)/Xsession.in $(DESTDIR)$(configfilesdir)
	$(INSTALL) -m 644 $(srcdir)/Xclients.in $(DESTDIR)$(configfilesdir)

install-data-local:
if INSTALLPAM
	$(INSTALL) -d $(DESTDIR)$(PAMDIR)
	$(INSTALL) -m 644 $(srcdir)/wdm.pamd $(DESTDIR)$(PAMDIR)/wdm
else
	
endif

uninstall-hook:
	rm -fr $(DESTDIR)$(configfilesdir)/authdir

dist-hook:
	cp -p *.in $(distdir)
	cp -p *.orig $(distdir)
