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

