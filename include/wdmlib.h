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
 */
#ifndef _WDMLIB_H
#define _WDMLIB_H

#include <WINGs/WINGs.h>
#include <WINGs/WUtil.h>

#include <stdio.h>
#include <sys/socket.h>

#ifndef __GNUC__
#define __attribute__(x)
#endif

typedef Bool (WDMChecker)(WMPropList *, void *, void *);

typedef struct _WDMArraySpec
{
	WDMChecker *checker;
	void *data;
	WMFreeDataProc *destructor;
	Bool addnull;
} WDMArraySpec;

typedef struct _WDMDictionaryStruct
{
	char *key;
	WDMChecker *checker;
	void *data;
	size_t offset;
} WDMDictionaryStruct;

typedef struct _WDMDictionarySpec
{
	size_t size;
	WDMDictionaryStruct *fields;
} WDMDictionarySpec;

extern Bool WDMCheckPLBool(WMPropList *pl, void *def, void *target);
extern Bool WDMCheckPLString(WMPropList *pl, void *def, void *target);
extern Bool WDMCheckPLArray(WMPropList *pl, void *def, void *target);
extern Bool WDMCheckPLDictionary(WMPropList *pl, void *def, void *target);
extern Bool WDMCheckPLStringOrArray(WMPropList *pl, void *def, void *target);

#define WDM_LEVEL_PANIC	   0
#define WDM_LEVEL_ERROR	   1
#define WDM_LEVEL_WARNING  2
#define WDM_LEVEL_INFO     3
#define WDM_LEVEL_DEBUG    4

extern int WDMLogLevel(int level);
extern FILE *WDMLogStream(FILE *debugfile);
extern void WDMUseSysLog(const char *ident, int facility);
extern void WDMLogMessage(int level, char *fmt, ...) __attribute__((format(printf, 2, 3)));
extern void WDMDebug(char *fmt, ...) __attribute__((format(printf, 1, 2)));
extern void WDMInfo(char *fmt, ...) __attribute__((format(printf, 1, 2)));
extern void WDMWarning(char *fmt, ...) __attribute__((format(printf, 1, 2)));
extern void WDMError(char *fmt, ...) __attribute__((format(printf, 1, 2)));
extern void WDMPanic(char *fmt, ...) __attribute__((noreturn, format(printf, 1, 2)));

extern void *WDMSockaddrGetPort(struct sockaddr *from, int *len);
extern void *WDMSockaddrGetAddr(struct sockaddr *from, int *len);
extern char *WDMGetHostName(struct sockaddr *from);
extern char *WDMGetHostAddr(struct sockaddr *from);

#endif

