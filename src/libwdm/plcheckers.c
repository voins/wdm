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
 * plcheckers.c: some functions to check correctness of proplist
 *		 used ad configuration data.
 */
#include <wdmlib.h>
#include <stddef.h>
#include <strings.h>

/** @brief check if proplist is correct boolean value
 *
 * If pl is string and has value "yes", than True is returned
 * If pl is string and has value "no", then False is returned
 * In all other cases defval is returned.
 *
 * This function is not signalling any error.
 */
Bool
WDMCheckPLBool(WMPropList *pl, Bool defval)
{
	Bool value = defval;
	char *text = NULL;

	if(pl && WMIsPLString(pl))
	{
		text = WMGetFromPLString(pl);
		if(!strcasecmp(text, "yes"))
		{
			value = True;
		}
		else if(!strcasecmp(text, "no"))
		{
			value = False;
		}
	}

	return value;
}

/** @brief check if proplist is a string
 *
 * If pl is a string it will be duplicated with 
 * wstrdup and returned. In all other cases defval
 * is returned. defval is also duplicated.  It's 
 * caller responsibility to wfree that space.
 *
 * If defval is NULL it is not duplicated.
 */
char *
WDMCheckPLString(WMPropList *pl, char *defval)
{
	char *value = defval;

	if(pl && WMIsPLString(pl))
	{
		value = WMGetFromPLString(pl);
	}

	return value ? wstrdup(value) : value;
}

/** @brief check if proplist is array and is correct
 *
 * If pl is an array all elements of it will be passed
 * through check function, and appended to newly created
 * array. Note that, array being returned is not PropList
 * array, but ordinary WMArray.
 *
 * check function should check correctness of array element
 * and return value that should be stored in resulting array.
 * data is passed to check function as second parameter.
 *
 * If check function allocates space for returned data it will
 * not be freed. Use WDMCheckPLArrayWithDestructor for that.
 */
WMArray *
WDMCheckPLArray(WMPropList *pl, void *(*check)(void *, void *), void *data)
{
	return WDMCheckPLArrayWithDestructor(pl, NULL, check, data);
}

/** @brief check if proplist is array and is correct
 *
 * This function behaves exactly the same as WDMCheckPLArray with
 * one exception. It takes additional destructor parameter, that
 * will be used to free space allocated by check function.
 * It can help prevent memory leaks.
 */
WMArray *
WDMCheckPLArrayWithDestructor(
	WMPropList *pl, WMFreeDataProc *destructor,
	void *(*check)(void *, void *), void *data)
{
	WMArray *value = NULL;
	void *entry = NULL;
	int i, count;

	if(pl && WMIsPLArray(pl))
	{
		count = WMGetPropListItemCount(pl);
		value = WMCreateArrayWithDestructor(count, destructor);

		for(i = 0; i < count; ++i)
		{
			entry = (*check)(WMGetFromPLArray(pl, i), data);
			if(!entry)
			{
				WMFreeArray(value);
				value = NULL;
				break;
			}
			WMAddToArray(value, entry);
		}
	}

	return value;
}

