/*
 * wdm - WINGs display manager
 * Copyright (C) 2003 Alexey Voinov <voins@voins.program.ru>
 * Copyright (C) 1998 Gene Czarcinski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * TestLogin.c: simple test for wdmLogin program
 */


/*
 * This is a simple test program to test the interface to
 * the external Login program rather than doing the whole
 * xdm thing.  This program is part of the Gdm package but
 * is NOT an installed program -- it is only used in development
 * and testing of "new" external Login programs.
 *
 * This program implements the external interface of
 * Gdm-greet.c
 *
 * The interface to the external Login is based on (and some code
 * munged from) the "xdm-extgreet" package
 * Copyright 1998 by Tom Rothamel.
 *
 * Gene Czarcinski, August, 1998.
 *
 ************************************************************************
 * The interface protocol:
 *
 *      A) xdm -> Login
 *              1. SIGTERM to terminate sucessfully
 *              2. SIGUSR1 to indicate failure; retry
 *              3. "variables" which are needed by Login
 *                 are passes as command-line arguments (that is,
 *                 as argc, *argv[]).  This has been used to make
 *                 it simple for Login. The values to be passed
 *                 can be set as Xresources
 *
 *      B) xdm <- Login ... This is performed via a pipe with
 *         xdm/greet doing reads and Login doing writes.  The
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
 *                      4 - exit (added) - exit Gdm/xdm
 *              5. The extension byte is followed by a string.
 *              6. The string is follwed by another extension byte
 *                 or a zero to indicate end-of-data.  Normally, there
 *                 will be only one non-zero extension code but
 *                 the protocol does not limit this.
 *
 *************************************************************************
 * The above protocol was defined by Tom Rothamel.  Additions include
 * extension codes other than 0 or 1, and passing args to Login.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <malloc.h>
#include <syslog.h>
#include <wdmlib.h>
#include <unistd.h>

#define forever 1

extern char **environ;

/* some testing data */

static char     LoginName[] = "gene";  /* any other user is not defined */
static char     LoginPswd[] = "Testing";

static char     *ExternalLogin = NULL;
static char     *ExternalName  = NULL;

/* this needs to be defined for the read routes */

void read_error(char *msg)
{
    WDMPanic("Pipe I/O Testing error: %s\n", msg);
}

/* This file is Copyright 1998 Tom Rothamel. It's under the Gnu Public   *
 * license, see the file COPYING for details.                            */


void guaranteed_read(int fd, char *buf, size_t count) {
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

        read_error("guaranteed_read failed.");
}

unsigned char readuc(int fd) {
        unsigned char uc;

        guaranteed_read(fd, (char *)&uc, sizeof(unsigned char));
        return uc;
}

char *readstring(int fd) {
        int len;
        char *buf;

        len = (int) readuc(fd);
        buf = malloc(len + 1);
        if(!buf) read_error("malloc of string failed.");
        guaranteed_read(fd, buf, len);
        buf[len] = 0;

        return buf;
}

/* the simple test routine */

int main (int argc, char *argv[])
{
        int  pid, filedescriptor[2], extcode=0, notdone = 0, i;
        char *username, *userpswd, *xsession=NULL, *exitstr=NULL;
	char **nargv;

#if 0
	FILE *f;
	if((f = fopen("TestLogin.log", "w")) == NULL)
		WDMPanic("cannot open log file");
	WDMLogStream(f);
#endif

#if 0
	WDMUseSysLog("TestLogin", LOG_USER);
#endif

#if 1
	WDMLogLevel(WDM_LEVEL_DEBUG);
#endif

        if (argc < 2)
		WDMPanic("oops -- need pgm to exec as first arg\n");

        ExternalLogin = argv[1];
        ExternalName  = strrchr(ExternalLogin,'/');
        if (ExternalName==NULL)
            ExternalName = ExternalLogin;
        else
            ExternalName++;
        WDMDebug("External Name: %s\n",ExternalName);
        WDMInfo("Testing: %s\n",ExternalLogin);

        pipe(filedescriptor);

        pid = fork();

        switch(pid) {
                case -1:
                        WDMPanic("Cannot fork for external process\n");
                break;
                case 0: /* this is the child process */
                        close(filedescriptor[0]);
                        fcntl(filedescriptor[1], F_SETFD, 0); /*unset close-on-exec */

			nargv = wmalloc(sizeof(char*) * (argc + 1));
			memcpy(nargv, argv + 1, sizeof(char*) * (argc - 1));
			nargv[argc - 1] = wmalloc(25);
			sprintf(nargv[argc - 1], "-f%i", filedescriptor[1]);
			nargv[argc] = NULL;

                        execve(ExternalLogin, nargv, environ);
                        WDMPanic("Cannot exec %s\n", ExternalLogin);
                break;
        }
        WDMDebug("Continuing, child=%i\n",pid);
        close (filedescriptor[1]);  /* Just need reading on our side */

        while (forever) {
            username = readstring(filedescriptor[0]);
            WDMInfo("Got username: %s\n", username);
            userpswd = readstring(filedescriptor[0]);
            WDMInfo("Got userpswd: %s\n", userpswd);

            notdone = 1;
            while (notdone) {
                i = readuc(filedescriptor[0]);
                switch (i) {
                        case 0:
                                notdone=0;
                        break;
                        case 1:
                                if (xsession)
                                    free(xsession);
                                xsession = readstring(filedescriptor[0]);
                                extcode = 1;
                        break;
                        case 2:
                                if (exitstr)
                                    free(exitstr);
                                exitstr = readstring(filedescriptor[0]);
                                WDMInfo("2=reboot: %s\n", exitstr);
                                extcode = 2;
                        break;
                        case 3:
                                if (exitstr)
                                    free(exitstr);
                                exitstr = readstring(filedescriptor[0]);
                                WDMInfo("3=halt: %s\n", exitstr);
                                extcode = 3;
                        break;
                        case 4:
                                if (exitstr)
                                    free(exitstr);
                                exitstr = readstring(filedescriptor[0]);
                                WDMInfo("4=exit: %s\n", exitstr);
                                extcode = 4;
                        break;
                        default:
                                WDMWarning("bad extension code %i\n",i);
                                /* keep reading here, greet should abort */
                        break;
                }
            }
            if (xsession) {
                WDMInfo("xsession=%s\n", xsession);
                free(xsession);
                xsession=NULL;
            }
            if (exitstr) {
                WDMInfo("exitstr=%s\n", exitstr);
                free(exitstr);
                exitstr=NULL;
            }
            /* testing insists on a good username/pswd to terminate */
            if (((strcmp(username,LoginName)==0) &&
                 (strcmp(userpswd,LoginPswd)==0)) ||
                (extcode==4)) {
                    WDMInfo("success! Now terminate and exit\n");
                    wusleep(1000000);
                    kill(pid, SIGTERM);
                    break; /* get out of forever loop */
            }
            WDMError(" bad name or password; go around again\n");
            wusleep(1000000);
            kill(pid, SIGUSR1);

            free(username);
            free(userpswd);
        }
        WDMInfo("All Done, exiting\n");
        return 0;
}
