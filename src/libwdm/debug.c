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
 * debug.c: some functions to help output debug information
 */
#include <wdmlib.h>
#include <stdarg.h>
#include <syslog.h>
#include <stdlib.h>

static Bool use_syslog = False;

int
WDMLogLevel(int level)
{
	static int current_level = WDM_LEVEL_ERROR;

	if(level < 0)
		return current_level;

	return current_level = level;
}

FILE *
WDMLogStream(FILE *debugfile)
{
	static FILE* current_debugfile = NULL;

	if(!current_debugfile)
		current_debugfile = stderr;

	if(!debugfile)
		return current_debugfile;

	use_syslog = False;
	return current_debugfile = debugfile;
}

void
WDMUseSysLog(const char *ident, int facility)
{
	if(!ident) ident = "wdm";
	openlog(ident, 0, facility);
	use_syslog = True;
}

static int
WDMLevelToSyslog(int level)
{
	switch(level)
	{
		case WDM_LEVEL_DEBUG:
			return LOG_DEBUG;
		case WDM_LEVEL_INFO:
			return LOG_INFO;
		case WDM_LEVEL_WARNING:
			return LOG_WARNING;
		case WDM_LEVEL_ERROR:
			return LOG_ERR;
		case WDM_LEVEL_PANIC:
			return LOG_CRIT;
	}
	return LOG_DEBUG;
}

static void
WDMLogMessageRaw(int level, char *fmt, va_list args)
{
	if(WDMLogLevel(-1) >= level)
	{
		if(use_syslog)
		{
			vsyslog(WDMLevelToSyslog(level), fmt, args);
		}
		else
		{
			vfprintf(WDMLogStream(NULL), fmt, args);
			fflush(WDMLogStream(NULL));
		}
	}
}

void
WDMLogMessage(int level, char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	WDMLogMessageRaw(level, fmt, args);
	va_end(args);
}

void
WDMDebug(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	WDMLogMessageRaw(WDM_LEVEL_DEBUG, fmt, args);
	va_end(args);
}

void
WDMInfo(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	WDMLogMessageRaw(WDM_LEVEL_INFO, fmt, args);
	va_end(args);
}

void
WDMWarning(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	WDMLogMessageRaw(WDM_LEVEL_WARNING, fmt, args);
	va_end(args);
}

void
WDMError(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	WDMLogMessageRaw(WDM_LEVEL_ERROR, fmt, args);
	va_end(args);
}

void
WDMPanic(char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	WDMLogMessageRaw(WDM_LEVEL_PANIC, fmt, args);
	va_end(args);
	exit(EXIT_FAILURE);
}

