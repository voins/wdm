dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.53)
AC_INIT([wdm],[0.0],[voins@voins.program.ru])
AC_CONFIG_SRCDIR(install-sh)
AC_CONFIG_HEADERS(include/config.h)

AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_RANLIB

AC_ARG_VAR(XRDB_PATH, [Path to xrdb program from X])
AC_PATH_PROG(XRDB_PATH, xrdb)
AC_DEFINE_UNQUOTED(XRDB_PROGRAM, "$XRDB_PATH", [Value is a path for xrdb program])
AC_SUBST(XRDB_PATH)


AC_ARG_VAR(XCONSOLE, [Path to xconsole program])
AC_PATH_PROG(XCONSOLE, xconsole)
AC_SUBST(XCONSOLE)


AC_ARG_VAR(SHUTDOWN, [Path to shutdown program])
AC_PATH_PROG(SHUTDOWN,shutdown,,$PATH:/sbin:/usr/sbin)
AC_SUBST(SHUTDOWN)


AC_ARG_VAR(FAILSAFE_PATH, [Path to program, that is used as failsafe client])
AC_PATH_PROG(FAILSAFE_PATH, xterm)
AC_DEFINE_UNQUOTED(DEF_FAILSAFE_CLIENT, "$FAILSAFE_PATH", [Value is a path for program used as failsafe client])
AC_DEFINE_UNQUOTED(DEF_SESSION, "$FAILSAFE_PATH -ls", [Value is a command for default session])
AC_SUBST(FAILSAFE_PATH)


AC_ARG_VAR(DEF_SERVER, [Path to default X server])
AC_PATH_PROG(DEF_SERVER, X)
AC_DEFINE_UNQUOTED(DEF_SERVER_LINE, ":0 local $DEF_SERVER :0", [Value is default command to start X server])
AC_SUBST(DEF_SERVER)


AC_ARG_VAR(SYSTEM_SHELL, [Path to default shell])
AC_PATH_PROG(SYSTEM_SHELL, sh)
AC_PATH_PROG(SHELL_BASH,bash,,$PATH:/usr/local/bin)
if test "x$SYSTEM_SHELL" = "x"; then
	AC_MSG_ERROR(the program sh cannot be found)
fi
dnl hope this is correct:
if test "x$SHELL_BASH" = "x"; then
	SHELL_LOGIN="$SYSTEM_SHELL -"
else
	SHELL_LOGIN="$SHELL_BASH -login"
fi
AC_SUBST(SHELL_LOGIN)
AC_SUBST(SYSTEM_SHELL)



AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h sys/param.h syslog.h libintl.h)

AC_DECL_SYS_SIGLIST

AC_C_CONST
AC_TYPE_SIZE_T
AC_TYPE_PID_T

dnl if there is mkstemp use it instead of mktemp
AC_CHECK_FUNC(mkstemp, 
	[AC_DEFINE(HAVE_MKSTEMP, 1,
		[Define if you have mkstemp function to use it instead of mktemp.])])

dnl Password processing options
AC_ARG_ENABLE(pam,
	AC_HELP_STRING([--disable-pam], [don't use PAM for authentication]))

AC_ARG_ENABLE(shadow,
	AC_HELP_STRING([--disable-shadow], [don't use shadow for authentication]))

AC_ARG_WITH(pamdir,
	AC_HELP_STRING([--with-pamdir],[specify where pam directory is]),
	[PAMDIR="$withval"],
	[PAMDIR="/etc/pam.d"])
AC_SUBST(PAMDIR)

USE_PAM=no
if test "$enable_pam" != "no" -a "$enable_shadow" != "yes" \
	|| test "$enable_pam" = "yes" -a "$enable_shadow" = "yes"; then

	AC_CHECK_LIB(pam, main,
		[
			USE_PAM=yes
			PASSWDLIB="-lpam -ldl"
			AC_DEFINE_UNQUOTED(HAVE_PAM_LIB, 1, [Define if libpam is installed])
			AC_DEFINE(USE_PAM, 1, [Define if you want wdm to use pam for auth])],
		,-ldl)

fi

if test -z "$PASSWDLIB" \
	&& ( test "$enable_shadow" != "no" -a "$enable_pam" != "yes" \
	|| test "$enable_pam" = "yes" -a "$enable_shadow" = "yes" ); then

	AC_CHECK_LIB(shadow, main,[PASSWDLIB="-lshadow"])
	AC_CHECK_HEADERS(shadow.h)
fi

AC_SUBST(PASSWDLIB)
AC_SUBST(USE_PAM)

AC_MSG_CHECKING(for SELinux support)
AC_ARG_ENABLE(selinux,
	AC_HELP_STRING([--enable-selinux], [enable SELinux support]),
   [
   	AC_MSG_RESULT(yes)
      	AC_CHECK_LIB(selinux, is_selinux_enabled, [SELINUX_LIBS="-lselinux"
         	AC_DEFINE_UNQUOTED(HAVE_SELINUX_LIB, 1, [Define if libselinux is installed])
			AC_DEFINE(WITH_SELINUX, 1, [Define if you want wdm to be compiled with SELinux support])
            	SELINUX_CFLAGS="-DWITH_SELINUX -I/usr/include/selinux"
               ],
               [
			AC_MSG_WARN([libselinux not found, compiling without SELinux support])
            	])
   ],
   [
   	AC_MSG_RESULT(no)
   ])
AC_SUBST(SELINUX_LIBS)
AC_SUBST(SELINUX_CFLAGS)

            
AC_CHECK_LIB(util, main) dnl for FreeBSD
AC_CHECK_LIB(crypt, main) dnl for BSD and RHL
AC_CHECK_LIB(ucb, killpg) dnl for Solaris2.4

AC_ARG_ENABLE(debug,
	AC_HELP_STRING([--enable-debug],[enable debugging]),
	[
		CFLAGS="$CFLAGS -g -DDEBUG"
	])

AC_PATH_XTRA
XCFLAGS="$X_CFLAGS"
XLDFLAGS="$X_LIBS"
XLIBS="$X_PRE_LIBS -lXext -lX11 -lm $X_EXTRA_LIBS"

AC_ARG_ENABLE(xinerama,
	AC_HELP_STRING([--disable-xinerama],[explicit disable use of libXinerama]))

if test "$enable_xinerama" = "yes" -o -z "$enable_xinerama"; then
	AC_CHECK_LIB(Xinerama, XineramaIsActive,
	[
		XLIBS="$XLIBS -lXinerama"
		AC_DEFINE(HAVE_XINERAMA, 1, Define if Xinerama library is installed)
	],,[$XLDFLAGS $XLIBS])
fi

dnl		WINGs support
AC_CHECK_PROG(GETWINGSFLAGS,get-wings-flags,get-wings-flags,,[$PATH])
if test -z "$GETWINGSFLAGS"; then
	AC_MSG_ERROR(['get-wings-flags' command was not found in \$PATH, please upgrade your WindowMaker.])
fi

WCFLAGS=`get-wings-flags --cflags`
WLDFLAGS=`get-wings-flags --ldflags`
WLIBS=`get-wings-flags --libs`

AC_SUBST(XLIBS)
AC_SUBST(XCFLAGS)
AC_SUBST(XLDFLAGS)
AC_SUBST(WCFLAGS)
AC_SUBST(WLDFLAGS)
AC_SUBST(WLIBS)


dnl ===============================================
dnl prefix
dnl	handle this here so we can use it
dnl ===============================================

dnl First, check for prefix:
AC_MSG_CHECKING(for prefix)
if test "$prefix" = "NONE"; then
	prefix="/usr/local"
fi
AC_MSG_RESULT($prefix)

AC_MSG_CHECKING(for exec_prefix)
if test "$exec_prefix" = "NONE"; then
	exec_prefix=$prefix
fi
AC_MSG_RESULT($exec_prefix)

dnl ===============================================
dnl specify some WITH options for these settings
dnl ===============================================

AC_MSG_CHECKING(for logdir)
AC_ARG_WITH(logdir,
	AC_HELP_STRING([--with-logdir=PATH],[specify directory where logfile goes]))
if test "x$with_logdir" != "x"; then
	LOGDIR=$with_logdir
else
	LOGDIR="/var/log"
fi
AC_SUBST(LOGDIR)
AC_MSG_RESULT($LOGDIR)

AC_MSG_CHECKING(for runlockdir)
AC_ARG_WITH(runlockdir,
	AC_HELP_STRING([--with-runlockdir=PATH],[specify directory where run lock goes]))
if test "x$with_runlockdir" != "x"; then
	RUNDIR=$with_runlockdir
else
	RUNDIR="/var/run"
fi
AC_SUBST(RUNDIR)
AC_MSG_RESULT($RUNDIR)

AC_MSG_CHECKING(for defuserpath)
AC_ARG_WITH(defuserpath, 
	AC_HELP_STRING([--with-defuserpath=PATHlist],[specify list of paths for user sessions]))
if test "x$with_defuserpath" != "x"; then
	AC_DEFINE_UNQUOTED(DEF_USER_PATH,"$with_defuserpath", "Value is a list of paths for user sessions")
	AC_MSG_RESULT($with_defuserpath)
else
	AC_DEFINE(DEF_USER_PATH,"/usr/local/bin:/bin:/usr/bin")
	AC_MSG_RESULT([using default: /usr/local/bin:/bin:/usr/bin])
fi

AC_MSG_CHECKING(for defsystempath)
AC_ARG_WITH(defsystempath,
	AC_HELP_STRING([--with-defsystempath=PATHlist],[specify list of paths for superuser sessions]))
if test "x$with_defsystempath" != "x"; then
	AC_DEFINE_UNQUOTED(DEF_SYSTEM_PATH,"$with_defsystempath", "Value is a list of paths for superuser sessions")
	AC_MSG_RESULT($with_defsystempath)
else
	AC_DEFINE(DEF_SYSTEM_PATH,"/sbin:/usr/sbin:/bin:/usr/bin:/usr/local/bin")
	AC_MSG_RESULT([using default: /sbin:/usr/sbin:/bin:/usr/bin:/usr/local/bin])
fi

dnl user must use --with-wdmdir= rather than --sysconfigdir= to put it
dnl somewhere other than $prefix/etc
dnl
AC_MSG_CHECKING(if  wdmdir has been specified)
AC_ARG_WITH(wdmdir,
	AC_HELP_STRING([--with-wdmdir=PATH],[specify path for wdm directory]))
if test "x$with_wdmdir" != "x"; then
	DMDIR="$with_wdmdir"
	AC_MSG_RESULT([yes, using $DMDIR])
else
	DMDIR="$prefix/etc/wdm"
	AC_MSG_RESULT([no, using $DMDIR])
fi
AC_DEFINE_UNQUOTED(WDMDIR, "$DMDIR", [Value is a path to wdm directory])
AC_DEFINE_UNQUOTED(DEF_XDM_CONFIG, "$DMDIR/wdm-config", [Value is a path to default wdm config file])
AC_DEFINE_UNQUOTED(DEF_WDMLOGIN_CONFIG, "$DMDIR/wdmLogin-config", [Value is a path to default wdmLogin config file])
AC_DEFINE_UNQUOTED(DEF_CHOOSER, "$DMDIR/chooser", [Value is a path to default chooser program])
AC_DEFINE_UNQUOTED(DEF_AUTH_DIR, "$DMDIR/authdir", [Value is a path to default authdir])

AC_SUBST(DMDIR)

dnl Let the user specify a location for pixmap files.
dnl
AC_MSG_CHECKING(if gfxdir has been specified)
AC_ARG_WITH(gfxdir,
	AC_HELP_STRING([--with-gfxdir=PATH],[specify path for pixmaps directory]))
if test "x$with_gfxdir" != "x"; then
	GFXDIR="$with_gfxdir"
	AC_MSG_RESULT([yes, using $GFXDIR])
else
	GFXDIR="$prefix/share/pixmaps/wdm"
	AC_MSG_RESULT([no, using $GFXDIR])
fi
AC_DEFINE_UNQUOTED(WGFXDIR, "$GFXDIR", [Value is a path to the pixmaps directory])

AC_SUBST(GFXDIR)

AC_MSG_CHECKING(if  Logo pixmap specified)
AC_ARG_WITH(Logo,
	AC_HELP_STRING([--with-Logo=PIXMAP_FILENAME],[specify file in wdm/pixmaps for Logo]))
if test "x$with_Logo" != "x"; then
	PIXMAPFILE="$with_Logo"
	PIXMAPCOMMENT=""
	AC_MSG_RESULT($with_Logo)
else
	PIXMAPFILE="Eagle.xpm"
	PIXMAPCOMMENT="! "
	AC_MSG_RESULT(no)
fi
AC_SUBST(PIXMAPFILE)
AC_SUBST(PIXMAPCOMMENT)



AC_ARG_VAR(WMAKER_PATH, [Path to wmaker program])
AC_PATH_PROG(WMAKER_PATH,wmaker,,$PATH:/usr/local/bin)
if test "x$WMAKER_PATH" != "x"; then
	WMLIST=WindowMaker
else
	WMAKER_PATH=wmaker
fi
AC_SUBST(WMAKER_PATH)


AC_ARG_VAR(AFTERSTEP_PATH, [Path to afterstep program])
AC_PATH_PROG(AFTERSTEP_PATH,afterstep,,$PATH:/usr/local/bin)
if test "x$AFTERSTEP_PATH" != "x"; then
	WMLIST=$WMLIST:AfterStep
else
	AFTERSTEP_PATH=afterstep
fi
AC_SUBST(AFTERSTEP_PATH)


AC_ARG_VAR(BLACKBOX_PATH, [Path to blackbox program])
AC_PATH_PROG(BLACKBOX_PATH,blackbox,,$PATH:/usr/local/bin)
if test "x$BLACKBOX_PATH" != "x"; then
	WMLIST=$WMLIST:BlackBox
else
	BLACKBOX_PATH=blackbox
fi
AC_SUBST(BLACKBOX_PATH)


AC_ARG_VAR(ICEWM_PATH, [Path to icewm program])
AC_PATH_PROG(ICEWM_PATH,icewm,,$PATH:/usr/local/bin)
if test "x$ICEWM_PATH" != "x"; then
	WMLIST=$WMLIST:IceWm
else
	ICEWM_PATH=icewm
fi
AC_SUBST(ICEWM_PATH)


AC_ARG_VAR(XSESSION_PATH, [Path to xsession program])
AC_PATH_PROG(XSESSION_PATH,xsession,,$PATH:/usr/local/bin)
if test "x$XSESSION_PATH" != "x"; then
	WMLIST=$WMLIST:Xsession
else
	XSESSION_PATH=xsession
fi
AC_SUBST(XSESSION_PATH)


AC_PATH_PROG(STARTKDE_PATH,startkde,,$PATH:/usr/local/bin)
if test "x$STARTKDE_PATH" != "x"; then
	WMLIST=$WMLIST:KDE
else
	STARTKDE_PATH=startkde
fi
AC_SUBST(STARTKDE_PATH)


AC_ARG_WITH(winmgr,
	AC_HELP_STRING([--with-winmgr=NAME_OF_WM], [specify an arbitrary window manager]))
if test "x$with_winmgr" != "x"; then
	WINMGR_NAME=$with_winmgr
	AC_PATH_PROG(WINMGR_PATH,$WINMGR_NAME,,$PATH:/usr/local/bin)
	if test "x$WINMGR_PATH" = "x"; then
	AC_ARG_WITH(winmgr_path,
		AC_HELP_STRING([--with-winmgr_path=WINMGR_PATH], [specify path to your window manager]))
		if test "x$with_winmgr_path" != "x"; then
			WINMGR_PATH=$with_winmgr_path
		fi
	fi
	AC_MSG_CHECKING(for winmgr_path)
	if test "x$WINMGR_PATH" = "x"; then
		AC_MSG_ERROR([unable to locate $WINMGR_NAME, use --with-winmgr_path])
	fi
	AC_MSG_RESULT($WINMGR_PATH)
	WMLIST=$WMLIST:$WINMGR_NAME
fi
AC_SUBST(WINMGR_PATH)
AC_SUBST(WINMGR_NAME)


AC_MSG_CHECKING(if  window manager list specified)
AC_ARG_WITH(wmlist,
	AC_HELP_STRING([--with-wmlist=LIST_OF_WM],[specify list of window managers]))
if test "x$with_wmlist" != "x"; then
	WMLIST="$with_wmlist"
	AC_MSG_RESULT($with_wmlist)
	AC_MSG_WARN(make sure Xsession and Xclients support the above list)
else
	AC_MSG_RESULT([no, default to $WMLIST])
fi
AC_SUBST(WMLIST)


dnl 
dnl gettext tests (partially copy/paste from WindowMaker's configure.ac)
dnl
INTLIBS=""
AC_CHECK_FUNC(gettext, [HAVEGETTEXT="yes"],
	AC_CHECK_LIB(intl, gettext, [INTLIBS="-lintl" HAVEGETTEXT="yes"],
		INTLIBS=""))

AC_CHECK_PROG(XGETTEXT, xgettext, xgettext)
AC_CHECK_PROG(MSGFMT, msgfmt, msgfmt)

if test "x$XGETTEXT" != "x"; then
	if $XGETTEXT --help 2>&1 | grep illegal &> /dev/null; then
		echo "xgettext isn't GNU version"
		XGETTEXT=""
	fi
fi

I18N=no
if test "x$MSGFMT" != "x" -a "x$HAVEGETTEXT" != "x"; then
	AC_DEFINE(I18N, 1, [Define if you want internationalization support])
	I18N=yes
fi
AC_SUBST(I18N)

AC_ARG_WITH(nlsdir,
	AC_HELP_STRING([--with-nlsdir=PATH],[specify where locale stuff should go]))

if test "x$NLSDIR" = "x"; then
	if test "x$with_nlsdir" != "x"; then
		NLSDIR=$with_nlsdir
	else
		NLSDIR=$prefix/lib/locale
	fi
fi

AC_DEFINE_UNQUOTED(NLSDIR, "$NLSDIR", [where to look for locale files])
AC_SUBST(NLSDIR)
AC_SUBST(XGETTEXT)
AC_SUBST(MSGFMT)

dnl this is hackaround for libfontconfig bug
AC_ARG_WITH(fakehome,
	AC_HELP_STRING([--with-fakehome=PATH],[path to use as home when no HOME is present. [/var/run/wdm] ]))

FAKEHOME=/var/run/wdm
if test "x$with_fakehome" != "x"; then
	FAKEHOME=$with_fakehome
fi
AC_DEFINE_UNQUOTED(FAKEHOME, "$FAKEHOME", [what to use as HOME value])
AC_SUBST(FAKEHOME)

AC_ARG_ENABLE(aafont,
	AC_HELP_STRING([--enable-aafont],[enable support for antialiased fonts.]))
if test "$enable_aafont" = "yes"; then
	AC_DEFINE(USE_AA, 1, [Define if you want support for antialiased fonts])
fi

AC_ARG_ENABLE(exitlogin,
	AC_HELP_STRING([--enable-exitlogin],[enable ExitLogin command in wdmLogin.]))
if test "$enable_exitlogin" = "yes"; then
	AC_DEFINE(WITH_EXITLOGIN, 1, [Define if you want to have ExitLogin])
fi

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

