/* $Xorg: util.c,v 1.4 2001/02/09 02:05:41 xorgcvs Exp $ */
/*

Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/xdm/util.c,v 3.19 2001/12/14 20:01:24 dawes Exp $ */

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * util.c
 *
 * various utility routines
 */

# include   <dm.h>

#include <X11/Xmu/SysUtil.h>	/* for XmuGetHostname */

#ifdef X_POSIX_C_SOURCE
#define _POSIX_C_SOURCE X_POSIX_C_SOURCE
#include <signal.h>
#undef _POSIX_C_SOURCE
#else
#if defined(X_NOT_POSIX) || defined(_POSIX_SOURCE)
#include <signal.h>
#else
#define _POSIX_SOURCE
#include <signal.h>
#undef _POSIX_SOURCE
#endif
#endif
#if defined(__osf__) || defined(linux) || defined(__QNXNTO__) || defined(__GNU__) \
	|| (defined(IRIX) && !defined(IRIX4))
#define setpgrp setpgid
#endif

#include <wdmlib.h>

void
printEnv (char **e)
{
	while (*e)
		WDMDebug ("%s\n", *e++);
}


void
freeEnv (char **env)
{
    char    **e;

    if (env)
    {
    	for (e = env; *e; e++)
	    free (*e);
    	free (env);
    }
}

# define isblank(c)	((c) == ' ' || c == '\t')

char **
parseArgs (char **argv, char *string)
{
	char	*word;
	char	*save;
	char    **newargv;
	int	i;

	i = 0;
	while (argv && argv[i])
		++i;
	if (!argv) {
		argv = (char **) malloc (sizeof (char *));
		if (!argv) {
			WDMError("parseArgs: out of memory");
			return 0;
		}
	}
	word = string;
	for (;;) {
		if (!*string || isblank (*string)) {
			if (word != string) {
				newargv = (char **) realloc ((char *) argv,
					(unsigned) ((i + 2) * sizeof (char *)));
				save = malloc ((unsigned) (string - word + 1));
				if (!newargv || !save) {
					WDMError("parseArgs: out of memory");
					free ((char *) argv);
					if (save)
						free (save);
					return 0;
				} else {
				    argv = newargv;
				}
				argv[i] = strncpy (save, word, string-word);
				argv[i][string-word] = '\0';
				i++;
			}
			if (!*string)
				break;
			word = string + 1;
		}
		++string;
	}
	argv[i] = 0;
	return argv;
}

void
freeArgs (char **argv)
{
    char    **a;

    if (!argv)
	return;

    for (a = argv; *a; a++)
	free (*a);
    free ((char *) argv);
}

void
CleanUpChild (void)
{
#ifdef CSRG_BASED
	setsid();
#else
#if defined(SYSV) || defined(SVR4) || defined(__CYGWIN__)
#if !(defined(SVR4) && defined(i386)) || defined(SCO325)
	setpgrp ();
#endif
#else
	setpgrp (0, getpid ());
	sigsetmask (0);
#endif
#endif
#ifdef SIGCHLD
	(void) Signal (SIGCHLD, SIG_DFL);
#endif
	(void) Signal (SIGTERM, SIG_DFL);
	(void) Signal (SIGPIPE, SIG_DFL);
	(void) Signal (SIGALRM, SIG_DFL);
	(void) Signal (SIGHUP, SIG_DFL);
	CloseOnFork ();
}

static char localHostbuf[256];
static int  gotLocalHostname;

char *
localHostname (void)
{
    if (!gotLocalHostname)
    {
	XmuGetHostname (localHostbuf, sizeof (localHostbuf) - 1);
	gotLocalHostname = 1;
    }
    return localHostbuf;
}

SIGVAL (*Signal (int sig, SIGFUNC handler))(int)
{
#if !defined(X_NOT_POSIX) && !defined(__EMX__)
    struct sigaction sigact, osigact;
    sigact.sa_handler = handler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(sig, &sigact, &osigact);
    return osigact.sa_handler;
#else
    return signal(sig, handler);
#endif
}
