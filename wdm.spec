Name:    wdm
Version: 1.20
Release: 0
Copyright: Copyright 1998 Gene Czarcinski - GPL
Source0:  wdm-%{PACKAGE_VERSION}.tar.gz
Packager: Gene Czarcinski <genec@mindspring.com>
Group:    X11/XFree86
Requires: XFree86 => 3.3.2
Requires: WindowMaker => 0.17.5
Requires: libPropList
BuildRoot: /tmp/wdm-root
Summary: WINGs Display Manager

%description
wdm is a modification/enhancement of XFree86's xdm which 
provides a more flexible login panel while maintaining the 
basic xdm code to interface with pam, etc.

The basic code uses XFree86 xdm with RedHat patches to implement pam.

This package is automake/autoconf based rather than Imake.

Xdm manages one more more X displays, which may be on the 
local host or remote servers.  wdm is an enhancement of 
the xdm distributed as part of XFree86.  It
replaces the logon panel with an external
"greet" module (that is, a separate binary executable file).  This separate
executable file has been implemented using the WindowMaker WINGs widget
library and the WindowMaker wraster graphics library.

The interface to the external routine supports passing a parameter to the
Xsession script (such as failsafe which just starts an xterm window).  This
interface can be used (with a modified Xsession) to specify the window
manager to start.

Except for the pam configuration file which is /etc/pam.d/wdm, 
the install location prefix of /usr/X11R6 is used.  wdm's configuration
directory is /etc/X11/wdm.

Note:  This rpm has been compiled assuming that the path for the
wmaker program is /usr/bin/wmaker and the path for the afterstep
program is /usr/X11R6/bin/afterstep.  If this is incorrect for your
system, then the file /etc/X11/wdm/Xclients must be modified to reflect
the correct locations.

Note:  Additional window managers can be added by modifying 
the wdmWm Xresources in the /etc/X11/wdm/wdm-config file and changing
the /etc/X11/wdm/Xclients file to start the added window managers.

Note: The rpm binary install process runs a post install script to
find window managers on the install system and update the wdm-config
and Xclients files.  Although this shell script can do a reasonable job,
there is nothing like editing these files to tailor them to a
particular system.  This script can be run manually if new window
managers are installed.

%changelog
* Sat Sep 13 1998 Gene Czarcinski <genec@mindspring.com>
  - create wdm 1.0
  - includes running wdmReconfig as postinstall process

%prep
%setup

%build
CFLAGS="${RPM_OPT_FLAGS}" ./configure \
	    --enable-pam \
	    --prefix=/usr/X11R6 \
	    --with-wdmdir=/etc/X11/wdm
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT  install

%post
/etc/X11/wdm/wdmReconfig

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc AUTHORS COPYING ChangeLog INSTALL NEWS README README.pam TODO
/etc/pam.d/wdm
/usr/X11R6/bin/wdm
/usr/X11R6/bin/wdmLogin
/usr/X11R6/man/man1/*
/etc/X11/wdm/*

