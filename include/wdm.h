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
 * wdm.h: header file with prototypes for wdm specific functions.
 */
#ifndef _WDM_H
#define _WDM_H

/* from src/wdm/loghelpers.c */
extern char *WDMLogMessages(int level, char *buffer, int n);
extern void WDMBufferedLogMessages(int level, char *buffer, int n);
extern int WDMRedirectFileToLog(int level, pid_t pid, int fd);
extern void WDMRedirectStderr(int level);

#endif /* _WDM_H */

