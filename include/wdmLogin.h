/*
 * wdm - WINGs display manager
 * Copyright (C) 2003 Alexey Voinov <voins@voins.program.ru>
 * Copyright (C) 1998 Gene Czarcinski
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
 * wdmLogin.h: header file for wdmLogin program
 */
#ifndef _WDMLOGIN_H
#define _WDMLOGIN_H

#include <wdmconfig.h>
#include <wdmlib.h>

typedef struct _WDMLoginConfig
{
	WMRect geometry;
#ifdef USE_AA
	Bool aaenabled;
	Bool multibyte;
#endif
} WDMLoginConfig;

extern WDMLoginConfig *LoadConfiguration(char *configFile);

#endif /* _WDMLOGIN_H */

