# automake input for creating wdm from source


# Note: The parameters for compile are a hack but created by seeing
#	what the Makefiles created by xmkmf -a produced.

# Note: chooser is no longer compiled.  It was not being installed anyway.

SUBDIRS = config pixmaps

bin_PROGRAMS = wdmLogin wdm

noinst_PROGRAMS = TestLogin

noinst_HEADERS = \
		dm.h \
		greet.h \
		wdmconfig.h

###config.h###
config.h:
	ln -sf ../config.h config.h

###wdmLogin###
wdmLogin_SOURCES = Login.c

wdmLogin_LDADD =  $(XLFLAGS) \
	$(GFXLIBS) \
	$(XLIBS)

wdmLogin_CFLAGS = -Wall $(X_CFLAGS) $(GFXFLAGS) $(DFLAGS)

Login.o: Login.c wdmconfig.h $(srcdir)/Makefile
	$(COMPILE) $(wdmLogin_CFLAGS) -c $<


###TestLogin###
TestLogin_SOURCES = TestLogin.c

TestLogin_CFLAGS = -Wall $(X_CFLAGS) $(DFLAGS)

TestLogin.o: TestLogin.c $(srcdir)/Makefile
	$(COMPILE) $(TestLogin_CFLAGS) -c $<


###wdm###
xdm_sources = \
		access.c \
		auth.c \
		choose.c \
		daemon.c \
		dm.c \
		dpylist.c \
		error.c \
		file.c \
		genauth.c \
		mitauth.c \
		netaddr.c \
		policy.c \
		protodpy.c \
		reset.c \
		resource.c \
		server.c \
		session.c \
		socket.c \
		streams.c \
		util.c \
		verify.c \
		xdmcp.c

xdm_objects = \
		access.o \
		auth.o \
		choose.o \
		daemon.o \
		dm.o \
		dpylist.o \
		error.o \
		file.o \
		genauth.o \
		mitauth.o \
		netaddr.o \
		policy.o \
		protodpy.o \
		reset.o \
		resource.o \
		server.o \
		session.o \
		socket.o \
		streams.o \
		util.o \
		verify.o \
		xdmcp.o

BUILT_SOURCES = $(xdm_sources)

# GFXFLAGS and GFXLIBS are not normally needed but in case the
# use needs to specify some non-standard path such as /usr/local/lib

wdm_LDADD =  \
	$(xdm_objects) \
	$(XLFLAGS) \
	$(GFXLIBS) \
	$(X_PRE_LIBS) \
	$(LIBCRYPT) $(PASSWDLIB) \
	$(LIBSOCKET) $(LIBUCB) $(LIBUTIL) \
	$(XDM_EXTRA_LIBS) \
	$(XLIBS)

wdm_SOURCES = Greet.c 

wdm_CFLAGS = $(X_CFLAGS) $(GFXFLAGS) $(DFLAGS)

Greet.o: Greet.c LinksDone $(xdm_objects) $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c Greet.c


resource.c: LinksDone

resource.o: resource.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c resource.c 

verify.c: LinksDone

verify.o: verify.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

auth.c: LinksDone

auth.o: auth.c dm.h $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

daemon.c: LinksDone

daemon.o: daemon.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

server.c: LinksDone

server.o: server.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

dpylist.c: LinksDone

dpylist.o: dpylist.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

dm.c: LinksDone

dm.o: dm.c dm.h $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

error.c: LinksDone

error.o: error.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

file.c: LinksDone

file.o: file.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

netaddr.c: LinksDone

netaddr.o: netaddr.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

reset.c: LinksDone

reset.o: reset.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

protodpy.c: LinksDone

protodpy.o: protodpy.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

policy.c: LinksDone

policy.o: policy.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

session.c: LinksDone

session.o: session.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

socket.c: LinksDone

socket.o: socket.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

streams.c: LinksDone

streams.o: streams.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

util.c: LinksDone

util.o: util.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

xdmcp.c: LinksDone

xdmcp.o: xdmcp.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

mitauth.c: LinksDone

mitauth.o: mitauth.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

genauth.c: LinksDone

genauth.o: genauth.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

access.c: LinksDone

access.o: access.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

choose.c: LinksDone

choose.o: choose.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

dm.h: LinksDone $(srcdir)/Makefile


greet.h: LinksDone $(srcdir)/Makefile


# The following should be links rather than copies.  Either
# work when compiling but dist does not work.  There is
# also BUILT_SOURCES but I cannot figure out the magic needed
# to make it do the compiles.  Oh well, this works for now.

LinksDone:
	cp -pf ../xdm/chooser.c chooser.c
	cp -pf ../xdm/dm.h dm.h
	patch -p1 <../patches/xdm-3.3.2-dm_h.patch
	cp -pf ../xdm/greet.h greet.h
	cp -pf ../xdm/auth.c auth.c
	cp -pf ../xdm/daemon.c daemon.c
	patch -p1 <../patches/xdm-3.3.2-irix1.patch
	cp -pf ../xdm/server.c server.c
	cp -pf ../xdm/dpylist.c dpylist.c
	cp -pf ../xdm/dm.c dm.c
	cp -pf ../xdm/error.c error.c
	cp -pf ../xdm/file.c file.c
	cp -pf ../xdm/netaddr.c netaddr.c
	cp -pf ../xdm/reset.c reset.c
	cp -pf ../xdm/protodpy.c protodpy.c
	cp -pf ../xdm/policy.c policy.c
	cp -pf ../xdm/session.c session.c
	patch -p1 <../patches/xdm-3.3.2-session.patch
	cp -pf ../xdm/socket.c socket.c
	cp -pf ../xdm/streams.c streams.c
	cp -pf ../xdm/util.c util.c
	patch -p1 <../patches/xdm-3.3.2-irix2.patch
	cp -pf ../xdm/xdmcp.c xdmcp.c
	cp -pf ../xdm/mitauth.c mitauth.c
	cp -pf ../xdm/genauth.c genauth.c
	cp -pf ../xdm/access.c access.c
	cp -pf ../xdm/choose.c choose.c
	cp -pf ../xdm/resource.c resource.c
	patch -p1 <../patches/xdm-3.3.2-resource2.patch
	cp -pf ../xdm/greeter/verify.c verify.c
	patch -p1 <../patches/xdm-3.3.2-pamwdm.patch
	touch LinksDone


CLEANFILES += chooser.c \
		dm.h \
		greet.h \
		$(xdm_sources) \
		LinksDone
