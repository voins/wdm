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
#include <string.h>

/*
 * Bool bool;
 * WDMCheckPLBool(pl, True, &bool);
 */
Bool
WDMCheckPLBool(WMPropList *pl, void *def, void *target)
{
	Bool *bool_target = (Bool*)target;
	char *text = NULL;

	WDMDebug("WDMCheckPLBool(%p, %p, %p)\n", (void *)pl, def, target);
	*bool_target = (Bool)def;
	if(pl && WMIsPLString(pl))
	{
		text = WMGetFromPLString(pl);
		if(!strcasecmp(text, "yes"))
		{
			*bool_target = True;
		}
		else if(!strcasecmp(text, "no"))
		{
			*bool_target = False;
		}
	}

	return True;
}

/*
 * char *text;
 * WDMCheckPLString(pl, NULL, &text);
 */
Bool
WDMCheckPLString(WMPropList *pl, void *def, void *target)
{
	char **charptr_target = (char**)target;
	char *value = (char*)def;

	WDMDebug("WDMCheckPLString(%p, %p, %p)\n", (void*)pl, def, target);
	if(pl && WMIsPLString(pl))
	{
		value = WMGetFromPLString(pl);
	}

	*charptr_target = value ? wstrdup(value) : value;

	return True;
}

/*
 * WMArray *array;
 * WDMCheckPLArray(pl, spec, &array);
 */
Bool
WDMCheckPLArray(WMPropList *pl, void *def, void *target)
{
	WMArray **array_target = (WMArray**)target;
	WDMArraySpec *spec = (WDMArraySpec*)def;
	void *entry = NULL;
	int i, count;

	WDMDebug("WDMCheckPLArray(%p, %p, %p)\n", (void*)pl, def, target);
	if(!pl || !WMIsPLArray(pl)) return False;

	count = WMGetPropListItemCount(pl);
	*array_target =
		WMCreateArrayWithDestructor(count, spec->destructor);

	for(i = 0; i < count; ++i)
	{
		if(!(*spec->checker)(
			WMGetFromPLArray(pl, i), spec->data, &entry))
		{
			WMFreeArray(*array_target);
			*array_target = NULL;
			return False;
		}
		if(spec->addnull == True || entry != NULL)
		{
			WMAddToArray(*array_target, entry);
		}
	}

	return True;
}

/*
 * struct *s;
 * WDMCheckPLDictionary(pl, spec, &s);
 */
Bool
WDMCheckPLDictionary(WMPropList *pl, void *def, void *target)
{
	WDMDictionarySpec *spec = (WDMDictionarySpec*)def;
	WDMDictionaryStruct *fields = spec->fields;
	void **data = (void**)target;
	WMPropList *key = NULL, *value = NULL;
	void *fresult = NULL;

	WDMDebug("WDMCheckPLDictionary(%p, %p, %p)\n", (void*)pl, def, target);
	if(!pl || !WMIsPLDictionary(pl)) return False;

	*data = (void*)wmalloc(spec->size);
	memset(*data, 0, spec->size);
	while(fields->key)
	{
		key = WMCreatePLString(fields->key);
		value = WMGetFromPLDictionary(pl, key);

		if((*fields->checker)(value, fields->data, &fresult))
			*((unsigned*)(*(unsigned char **)data + fields->offset))
				= (unsigned)fresult;

		WMReleasePropList(key);
		key = NULL;
		fields++;
	}
	return True;
}

/*
 * This function will check if pl is string or array of strings.
 * It always returns WMArray. In case of string, new array will be
 * created and that string will be added to it.
 * def is ignored here.
 */
Bool
WDMCheckPLStringOrArray(WMPropList *pl, void *def, void *target)
{
	char *text;
	WMArray **array_target = (WMArray**)target;
	static WDMArraySpec array_of_strings =
		{WDMCheckPLString, NULL, wfree, False};

	if(pl && WMIsPLString(pl))
	{
		if(WDMCheckPLString(pl, NULL, &text) && text)
		{
			*array_target = 
				WMCreateArrayWithDestructor(1, wfree);

			WMAddToArray(*array_target, text);

			return True;
		}
	}
	return WDMCheckPLArray(pl, &array_of_strings, target);
}

