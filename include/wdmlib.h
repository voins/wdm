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

extern int WDMDebugLevel(int level);
extern FILE *WDMDebugStream(FILE *debugfile);
extern void WDMDebug(char *fmt, ...);

#endif

