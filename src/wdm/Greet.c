/* $XConsortium: greet.c,v 1.41 94/09/12 21:32:49 converse Exp $ */
/* $XFree86: xc/programs/xdm/greeter/greet.c,v 3.1 1995/10/21 12:52:36 dawes Exp $ */
/*

Copyright (c) 1988  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * widget to get username/password
 *
 */

/*###################################################################*/
/*##                            Greet                              ##*/
/*##                                                               ##*/
/*##    This software is Copyright (C) 1998 by Gene Czarcinski.    ##*/
/*## This software falls under the GNU Public License. Please read ##*/
/*##              the COPYING file for more information            ##*/
/*##                                                               ##*/
/*## At the present time, PingServer is not implemented.           ##*/
/*##                                                               ##*/
/*###################################################################*/

/*
 ************************************************************************
 * The interface protocol between Greet and the external program:
 *
 *      A) wdm -> wdmLogin
 *              1. SIGTERM to terminate sucessfully
 *              2. SIGUSR1 to indicate failure; retry
 *              3. "variables" which are needed by Login
 *                 are passed as command-line arguments (that is,
 *                 as argc, *argv[]).  This has been used to make
 *                 it simple for Login. The values to be passed
 *                 can be set as Xresources.
 *
 *      B) wdm <- wdmLogin ... This is performed via a pipe with
 *         Greet doing reads and Login doing writes.  The
 *         stream of bytes follows the following protocol:
 *              1. A String is defined as a one byte length field
 *                 followed by a string of non-zero bytes.
 *              2. Transmission is ended with a single byte
 *                 with a zero value.
 *              3. Two strings are first transmitted which are
 *                 the username and password.
 *              4. This is followed by an extension byte.  The following
 *                 extension codes are supported:
 *                      0 - end of data (not really an extension code)
 *                      1 - xsession parameter (original)
 *                      2 - reboot (added) - shutdown -r now
 *                      3 - halt (added) - shutdown -h now
 *                      4 - exit (added) - exit wdm/xdm
 *              5. The extension byte is followed by a string.
 *              6. The string is followed by another extension byte
 *                 or a zero to indicate end-of-data.  Normally, there
 *                 will be only one non-zero extension code but
 *                 the protocol does not limit this.
 *
 *************************************************************************
 * The above protocol was defined by Tom Rothamel.  Additions include
 * extension codes other than 0 or 1, and passing args to Login. 
 */


#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <dm.h>
#include <greet.h>

/* wdm additions */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <limits.h>

#include <wdmlib.h>

extern Display  *dpy;

extern char *wdmLogin;          /* X resources (see resource.c) */
extern char *wdmWm;
extern char *wdmLogo;
extern char *wdmHelpFile;
extern char *wdmDefaultUser;
extern char *wdmDefaultPasswd;
extern char *wdmBg;
extern char *wdmReboot;
extern char *wdmHalt;
extern int   wdmRoot;
extern int   wdmVerify;
extern int   wdmAnimations;
extern char *wdmLocale;
extern char *wdmLoginConfig;
extern char *wdmCursorTheme;
extern int   wdmXineramaHead;

static int      pipe_filedes[2];
static char	name[128], password[128];
static char     xsessionArg[256], exitArg[256];

struct display *Save_d=NULL;

extern  char    **systemEnv();

/****  pipe I/O routines ****/

/** The following code was adapted from in.c by Tom Rothamel */

/* This file is Copyright 1998 Tom Rothamel. It's under the Gnu Public   *
 * License, see the file COPYING for details.                            */

static void guaranteed_read(int fd, char *buf, size_t count)
{
        int bytes_read;

        if (count == 0) {
                return;
        }


        while((bytes_read = read(fd, buf, count))>0) {
                count -= bytes_read;
                buf += bytes_read;

                if (count == 0) {
                        return;
                }
        }

        WDMError("Greet: guarenteed_read error, UNMANAGE DISPLAY\n");
        WDMError("Greet: pipe read error with %s\n", wdmLogin);
        SessionExit (Save_d, RESERVER_DISPLAY, FALSE);  /* this exits */
        exit (UNMANAGE_DISPLAY);                        /* should not happen */
}

static unsigned char readuc(int fd)
{
        unsigned char uc;

        guaranteed_read(fd, (char *)&uc, sizeof(unsigned char));
        return uc;
}

static char *readstring(int fd, char *buf)
{
        int len;

        len = (int) readuc(fd);
        guaranteed_read(fd, buf, len);
        buf[len] = 0;

        return buf;
}

/**** end of pipe I/O routines ****/

static int InitGreet (struct display *d)
{
    int pid;

    WDMDebug("Greet display=%s\n", d->name);

    pipe(pipe_filedes);

    pid = fork();

    if (pid == -1) {            /* error */
        WDMError("Greet cannot fork\n");
        exit(RESERVER_DISPLAY);
    }
    if (pid == 0) {             /* child */
        char **env = NULL;
	char *argv[20];
	int argc = 1; /* argc = 0 is for command itself */

        close(pipe_filedes[0]);
        fcntl(pipe_filedes[1], F_SETFD, 0); /* Reset close-on-exec (just in case) */

        env = (char **)systemEnv(d, (char*)NULL, FAKEHOME);

	if(*wdmLocale)
		env = WDMSetEnv(env, "LANG", wdmLocale);

	if(*wdmCursorTheme)
		env = WDMSetEnv(env, "XCURSOR_THEME", wdmCursorTheme);

	if((argv[0] = strrchr(wdmLogin, '/')) == NULL)
		argv[0] = wdmLogin;
	else
		argv[0]++;

	argv[argc++] = wstrconcat("-d", d->name);
	if(*wdmWm)
		argv[argc++] = wstrconcat("-w", wdmWm);
	if(*wdmLogo)
		argv[argc++] = wstrconcat("-l", wdmLogo);
	if(*wdmHelpFile)
		argv[argc++] = wstrconcat("-h", wdmHelpFile);
	if(*wdmDefaultUser)
		argv[argc++] = "-u";
	if(*wdmBg)
		argv[argc++] = wstrconcat("-b", wdmBg);
	if(*wdmLoginConfig)
		argv[argc++] = wstrconcat("-c", wdmLoginConfig);
	if(wdmAnimations)
		argv[argc++] = "-a";
	if(wdmXineramaHead)
	{
		argv[argc] = wmalloc(25); /* much more than length of 64bit integer 
					     converted to string, but it still a hack */
		sprintf(argv[argc++], "-x%i", wdmXineramaHead);
	}
	argv[argc] = wmalloc(25); 
	sprintf(argv[argc++], "-f%i", pipe_filedes[1]);

	argv[argc++] = NULL;
        execve(wdmLogin, argv, env);

        WDMError("Greet cannot exec %s\n", wdmLogin);
        exit(RESERVER_DISPLAY);
    }

    close(pipe_filedes[1]);
    RegisterCloseOnFork(pipe_filedes[0]);
    return pid;
}

static void CloseGreet (int pid)
{

/*    UnsecureDisplay (d, dpy);*/

    kill(pid,SIGTERM);

/*    ClearCloseOnFork (XConnectionNumber (dpy));*/
/*    XCloseDisplay (dpy);*/
    WDMDebug("Greet connection closed\n");
}

static int Greet (struct display *d, struct greet_info *greet)
{
    int code = 0, done = 0, extension_code=0;

    readstring(pipe_filedes[0], name);  /* username */
    readstring(pipe_filedes[0], password);

    xsessionArg[0] = '\0';
    while (!done) {
        extension_code = readuc(pipe_filedes[0]);
        switch (extension_code) {
           case 0:      /*end of data*/
                done = 1;
           break;
           case 1:      /*xsession parameter*/
                readstring(pipe_filedes[0], xsessionArg);
           break;
           case 2:      /*reboot*/
                readstring(pipe_filedes[0], exitArg);
                code = 2;
           break;
           case 3:      /*halt*/
                readstring(pipe_filedes[0], exitArg);
                code = 3;
           break;
           case 4:      /*exit*/
                readstring(pipe_filedes[0], exitArg);
                code = 4;
           break;
           default:     /*????*/
                WDMError("Bad extension code from external program: %i\n",
				code);
                exit (RESERVER_DISPLAY);
           break;
        }
    }
    greet->name = name;
    greet->password = password;

    if (xsessionArg[0] == '\0')
        greet->string = NULL;
    else
        greet->string = xsessionArg;

    return code;
}

/**** This is the entry point from session.c ****/

greet_user_rtn GreetUser(struct display *d, Display **dpy, struct verify_info
                                *verify, struct greet_info *greet, struct dlfuncs *dlfuncs)
{
    int flag;
    int pid;
    int code;

    Save_d = d;                         /* hopefully, this is OK */

    *dpy = XOpenDisplay (d->name);      /* make sure we have the display */
    /*
     * Run the setup script - note this usually will not work when
     * the server is grabbed, so we don't even bother trying.
     */
    if (!d->grabServer)
        SetupDisplay (d);
    if (!*dpy) {
        WDMError("Cannot reopen display %s for greet window\n", d->name);
        exit (RESERVER_DISPLAY);
    }

    pid = InitGreet(d); /* fork and exec the external program */

    for (;;) {
        /*
         * Greet user, requesting name/password
         */
        code = Greet (d, greet);
        WDMDebug("Greet greet done: %s, pwlen=%i\n", name, strlen(password));

        if (code != 0)
        {
            WDMDebug("Greet: exit code=%i, %s\n", code, exitArg);
            if (wdmVerify || wdmRoot) {
                flag = False;
                if (Verify (d, greet, verify))
                    flag = True;
                if (wdmRoot && (strcmp(greet->name,"root")!=0))
                    flag = False;
            }
            else
                flag = True;
	    if (flag == True) {
                switch (code) {
                   case 2:              /* reboot */
                        CloseGreet (pid);
                        WDMInfo("reboot(%s) by %s\n", exitArg, name);
                        system(wdmReboot);
                        SessionExit (d, UNMANAGE_DISPLAY, FALSE);
                   break;
                   case 3:              /* halt */
                        CloseGreet (pid);
                        WDMInfo("halt(%s) by %s\n", exitArg, name);
                        system(wdmHalt);
                        SessionExit (d, UNMANAGE_DISPLAY, FALSE);
                   break;
                   case 4:              /* exit */
                        CloseGreet (pid);
                        WDMDebug("UNMANAGE_DISPLAY\n");
                        WDMInfo("%s exit(%s) by %s\n",
					PACKAGE_NAME, exitArg, name);
#if 0
                        SessionExit (d, UNMANAGE_DISPLAY, FALSE);
#else
                        WDMDebug ("Killing parent process %d\n", getppid());
                        kill (getppid(), SIGINT);
#endif
                   break;
                }
            }
            else {
                kill(pid,SIGUSR1);      /* Verify failed */
            }
        }
        else {
            /*
             * Verify user
             */
	    if ((! *greet->name) && *wdmDefaultUser)
	    {
		greet->name = wdmDefaultUser;
		greet->password = wdmDefaultPasswd;
	    }
            if (Verify (d, greet, verify))
                break;
            else
                kill(pid,SIGUSR1);      /* Verify failed */
        }
    }
    DeleteXloginResources (d, *dpy);
    CloseGreet (pid);
    WDMDebug("Greet loop finished\n");
    /*
     * Run system-wide initialization file
     */
    if (source (verify->systemEnviron, d->startup) != 0)
    {
        WDMDebug("Startup program %s exited with non-zero status\n",
                d->startup);
        SessionExit (d, OBEYSESS_DISPLAY, FALSE);
    }

    return Greet_Success;
}

