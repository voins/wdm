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
 * names.c: functions to access host names and addresses
 */
#include <WINGs/WUtil.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void *
WDMSockaddrGetPort(struct sockaddr *from, int *len)
{
	switch(from->sa_family)
	{
		case AF_INET:
			if(len) *len = sizeof(in_port_t);
			return (void *)
				&(((struct sockaddr_in *)from)->sin_port);
			break;
		case AF_INET6:
			if(len) *len = sizeof(in_port_t);
			return (void *)
				&(((struct sockaddr_in6 *)from)->sin6_port);
			break;
		default:
			if(len) *len = 0;
			return NULL;
	}
}

void *
WDMSockaddrGetAddr(struct sockaddr *from, int *len)
{
	switch(from->sa_family)
	{
		case AF_INET:
			if(len) *len = sizeof(struct in_addr);
			return (void *)
				&(((struct sockaddr_in *)from)->sin_addr);
			break;
		case AF_INET6:
			if(len) *len = sizeof(struct in6_addr);
			return (void *)
				&(((struct sockaddr_in6 *)from)->sin6_addr);
			break;
		default:
			if(len) *len = 0;
			return NULL;
	}
}

char *
WDMGetHostName(struct sockaddr *from)
{
	struct hostent *he;
	void *addr;
	int len;

	addr = WDMSockaddrGetAddr(from, &len);
	if((he = gethostbyaddr(addr, len, from->sa_family))==NULL)
		return NULL;

	return wstrdup(he->h_name);
}

char *
WDMGetHostAddr(struct sockaddr *from)
{
	char ipbuf[128]; /* FIXME: I don't like fixed size buffers */

	inet_ntop(from->sa_family,
		WDMSockaddrGetAddr(from, NULL), ipbuf, sizeof(ipbuf));

	return wstrdup(ipbuf);
}

