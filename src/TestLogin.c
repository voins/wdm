/*###################################################################*/
/*##                          LoginTest                            ##*/
/*##                                                               ##*/
/*##    This software is Copyright (C) 1998 by Gene Czarcinski.    ##*/
/*## This software falls under the GNU Public License. Please read ##*/
/*##              the COPYING file for more information            ##*/
/*###################################################################*/

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

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <malloc.h>


#define forever 1

extern char **environ;

/* some testing data */

static char     LoginName[] = "gene";  /* any other user is not defined */
static char     LoginPswd[] = "Testing";

static char     *ExternalLogin = NULL;
static char     *ExternalName  = NULL;

static char     *Arg1 = "";
static char     *Arg2 = "";
static char     *Arg3 = "";
static char     *Arg4 = "";
static char     *Arg5 = "";
static char     *Arg6 = "";
static char     *Arg7 = "";
static char     *Arg8 = "";
static char     *Arg9 = "";

/* this needs to be defined for the read routes */

void read_error(char *msg)
{
    fprintf(stderr,"Pipe I/O Testing error: %s\n",msg);
    exit(4);
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
        
        guaranteed_read(fd, &uc, sizeof(unsigned char));
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

        if (argc < 2) {
                fprintf(stderr,"oops -- need pgm to exec as first arg\n");
                exit(1);
        }
        
        ExternalLogin = argv[1];
        ExternalName  = strrchr(ExternalLogin,'/');
        if (ExternalName==NULL)
            ExternalName = ExternalLogin;
        else
            ExternalName++;
        fprintf(stderr,"External Name: %s\n",ExternalName);
        
        fprintf(stderr,"Testing: %s\n",ExternalLogin);
        
        if (argc > 2) Arg1 = argv[2];
        if (argc > 3) Arg2 = argv[3];
        if (argc > 4) Arg3 = argv[4];
        if (argc > 5) Arg4 = argv[5];
        if (argc > 6) Arg5 = argv[6];
        if (argc > 7) Arg6 = argv[7];
        if (argc > 8) Arg7 = argv[8];
        if (argc > 9) Arg8 = argv[9];
        
        pipe(filedescriptor);
        
        pid = fork();
        
        switch(pid) {
                case -1:
                        fprintf(stderr,"Cannot fork for external process\n");
                        exit(2);
                break;
                case 0: /* this is the child process */
                        close(filedescriptor[0]);
                        dup2(filedescriptor[1], 3);
                        fcntl(3, F_SETFD, 0); /*unset close-on-exec */
                        execle(ExternalLogin,ExternalName,
                                Arg1, Arg2, Arg3, Arg4, Arg5,
                                Arg6, Arg7, Arg8, Arg9, NULL,
                                environ);
                        fprintf(stderr,"Cannot exec %s\n",ExternalLogin);
                        exit(3);
                break;
        }
        fprintf(stderr,"Continuing, child=%i\n",pid);
        close (filedescriptor[1]);  /* Just need reading on our side */
        
        while (forever) {
            username = readstring(filedescriptor[0]);
            fprintf(stderr,"Got username: %s\n",username);
            userpswd = readstring(filedescriptor[0]);
            fprintf(stderr,"Got userpswd: %s\n",userpswd);

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
                                fprintf(stderr,"2=reboot: %s\n",exitstr);
                                extcode = 2;
                        break;
                        case 3:
                                if (exitstr)
                                    free(exitstr);
                                exitstr = readstring(filedescriptor[0]);
                                fprintf(stderr,"3=halt: %s\n",exitstr);
                                extcode = 3;
                        break;
                        case 4:
                                if (exitstr)
                                    free(exitstr);
                                exitstr = readstring(filedescriptor[0]);
                                fprintf(stderr,"4=exit: %s\n",exitstr);
                                extcode = 4;
                        break;
                        default:
                                fprintf(stderr,"bad extension code %i\n",i);
                                /* keep reading here, greet should abort */
                        break;
                }
            }
            if (xsession) {
                fprintf(stderr,"xsession=%s\n",xsession);
                free(xsession);
                xsession=NULL;
            }
            if (exitstr) {
                fprintf(stderr,"exitstr=%s\n",exitstr);
                free(exitstr);
                exitstr=NULL;
            }
            /* testing insists on a good username/pswd to terminate */
            if (((strcmp(username,LoginName)==0) &&
                 (strcmp(userpswd,LoginPswd)==0)) ||
                (extcode==4)) {
                    fprintf(stderr,"sucsess! Now terminate and exit\n");
                    usleep(1000000);
                    kill(pid, SIGTERM);
                    break; /* get out of forever loop */
            }
            fprintf(stderr," bad name or password; go around again\n");
            usleep(1000000);
            kill(pid,SIGUSR1);
                
            free(username);
            free(userpswd);
        }
        fprintf(stderr,"All Done, exiting\n");
        return 0;
}
