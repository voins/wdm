# automake input for creating wdm from source


# Note: The parameters for compile are a hack but created by seeing
#	what the Makefiles created by xmkmf -a produced.

# Note: chooser is no longer compiled.  It was not being installed anyway.

SUBDIRS = pixmaps

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

wdmLogin_LDADD =  $(XLDFLAGS) \
	$(GFXLDFLAGS) $(GFXLIBS) \
	$(XLIBS)

wdmLogin_CFLAGS = -Wall $(XCFLAGS) $(GFXCFLAGS)

Login.o: Login.c wdmconfig.h $(srcdir)/Makefile
	$(COMPILE) $(wdmLogin_CFLAGS) -c $<


###TestLogin###
TestLogin_SOURCES = TestLogin.c

TestLogin_CFLAGS = -Wall $(XCFLAGS)

TestLogin_LDADD = $(XLDFLAGS)

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
	$(XLDFLAGS) \
	$(GFXLDFLAGS) \
	$(GFXLIBS) \
	$(LIBCRYPT) $(PASSWDLIB) \
	$(LIBSOCKET) $(LIBUCB) $(LIBUTIL) \
	-lXmu -lXau -lXdmcp \
	$(XLIBS)

wdm_SOURCES = Greet.c 

wdm_CFLAGS = $(XCFLAGS) $(GFXCFLAGS)

Greet.o: Greet.c $(xdm_objects) $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c Greet.c


resource.o: resource.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c resource.c 

verify.o: verify.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

auth.o: auth.c dm.h $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

daemon.o: daemon.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

server.o: server.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

dpylist.o: dpylist.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

dm.o: dm.c dm.h $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

error.o: error.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

file.o: file.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

netaddr.o: netaddr.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

reset.o: reset.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

protodpy.o: protodpy.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

policy.o: policy.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

session.o: session.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

socket.o: socket.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

streams.o: streams.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

util.o: util.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

xdmcp.o: xdmcp.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

mitauth.o: mitauth.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

genauth.o: genauth.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

access.o: access.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

choose.o: choose.c $(srcdir)/Makefile
	$(COMPILE) $(wdm_CFLAGS) -c $<

dm.h: $(srcdir)/Makefile


greet.h: $(srcdir)/Makefile



CLEANFILES += chooser.c \
		dm.h \
		greet.h \
		$(xdm_sources)
