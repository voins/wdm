/*
 * wdm - WINGs display manager
 * Copyright (C) 2003 Alexey Voinov <voins@voins.program.ru>
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
 * loghelpers.c: helper functions for intecepting messages in stderr
 *               and redirecting them to standard logging functions.
 */
#include <wdmlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Function that treats buffer as set of lines separated with '\n'
 * These lines will be directed to WDMLogMessage function with
 * specified message level. n is a number of actual characters in buffer.
 *
 * Returned value is pointer to string that is left unterminated with '\n'
 */
char *
WDMLogMessages(int level, char *buffer, int n)
{
	char *tmpmsg, *pos;

	while((pos = memchr(buffer, '\n', n)) != NULL)
	{
		tmpmsg = wmalloc(pos - buffer + 2);
		strncpy(tmpmsg, buffer, pos - buffer + 1);
		tmpmsg[pos - buffer + 1] = '\0';
		WDMLogMessage(level, "%s", tmpmsg);
		wfree(tmpmsg);

		n -= pos - buffer + 1;
		buffer = pos + 1;
	}

	return buffer;
}

void
WDMBufferedLogMessages(int level, char *buffer, int n)
{
	static char *old = NULL;
	static size_t oldn = 0;

	char *rest;

	old = wrealloc(old, oldn + n);
	memcpy(old + oldn, buffer, n);
	oldn += n;

	rest = WDMLogMessages(level, old, oldn);

	oldn -= rest - old;
	memmove(old, rest, oldn);
}

int
WDMRedirectFileToLog(int level, pid_t pid, int fd)
{
	fd_set set;
	int status;
	struct timeval tv;
	char buf[1024];
	int n;
	
	WDMDebug("logger started\n");
	while(waitpid(pid, &status, WNOHANG) == 0)
	{
		FD_ZERO(&set);
		FD_SET(fd, &set);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		if(select(fd + 1, &set, NULL, NULL, &tv) > 0)
		{
			n = read(fd, buf, sizeof(buf));
			if(n == -1)
				WDMError("error reading form pipe(stderr)\n");

			WDMBufferedLogMessages(level, buf, n);
		}
	}
	WDMBufferedLogMessages(level, "\n", 1);
	WDMDebug("logger stopped\n");

	return WEXITSTATUS(status);
}

void
WDMRedirectStderr(int level)
{
	int errpipe[2];
	pid_t childpid;
	int exitstatus;

	if(pipe(errpipe) == -1)
		WDMError("cannot create pipe. all stderr messages will go to stderr\n");

	childpid = fork();
	if(childpid == -1)
	{
		WDMError("fork failed. all stderr messages will go to stderr\n");
		close(errpipe[0]);
		close(errpipe[1]);
	}
	else if(childpid != 0)
	{
		/* parent, will read all messages from stderr and
		 * redirect it to log */
		close(errpipe[1]);
		exitstatus = WDMRedirectFileToLog(WDM_LEVEL_ERROR, childpid, errpipe[0]);
		close(errpipe[0]);
		exit(exitstatus);
	}

	/* child, will close read end of pipe and dup2
	 * write end of pipe to stderr */
	close(errpipe[0]);
	dup2(errpipe[1], 2);
}

