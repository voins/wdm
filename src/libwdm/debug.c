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

int
WDMDebugLevel(int level)
{
	static int current_level = 0;

	if(level < 0)
		return current_level;

	return current_level = level;
}

FILE *
WDMDebugStream(FILE *debugfile)
{
	static FILE* current_debugfile = NULL;

	if(!current_debugfile)
		current_debugfile = stderr;

	if(!debugfile)
		return current_debugfile;

	return current_debugfile = debugfile;
}

void
WDMDebug(char *fmt, ...)
{
	if(WDMDebugLevel(-1))
	{
		va_list args;
		va_start(args, fmt);
		vfprintf(WDMDebugStream(NULL), fmt, args);
		va_end(args);
		fflush(WDMDebugStream(NULL));
	}
}

