/* $Xorg: netaddr.c,v 1.4 2001/02/09 02:05:40 xorgcvs Exp $ */
/*

Copyright 1991, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/xdm/netaddr.c,v 3.6 2001/12/14 20:01:22 dawes Exp $ */

/*
 * xdm - X display manager
 *
 * netaddr.c - Interpretation of XdmcpNetaddr object.
 */

#include <dm.h>

#include <X11/X.h>		/* FamilyInternet, etc. */

#ifdef XDMCP

#include <dm_socket.h>

#ifdef UNIXCONN
#ifndef X_NO_SYS_UN
#ifndef Lynx
#include <sys/un.h>		/* struct sockaddr_un */
#else
#include <un.h>			/* struct sockaddr_un */
#endif
#endif
#endif

#include <wdmlib.h>

/* given an XdmcpNetaddr, returns the socket protocol family used,
   e.g., AF_INET */

int NetaddrFamily(XdmcpNetaddr netaddrp)
{
    return ((struct sockaddr *)netaddrp)->sa_family;
}


/* given an XdmcpNetaddr, returns a pointer to the TCP/UDP port used
   and sets *lenp to the length of the address
   or 0 if not using TCP or UDP. */

char * NetaddrPort(XdmcpNetaddr netaddrp, int *lenp)
{
    switch (NetaddrFamily(netaddrp))
    {
    case AF_INET:
	*lenp = 2;
	return (char *)&(((struct sockaddr_in *)netaddrp)->sin_port);
    default:
	*lenp = 0;
	return NULL;
    }
}


/* given an XdmcpNetaddr, returns a pointer to the network address
   and sets *lenp to the length of the address */

char * NetaddrAddress(XdmcpNetaddr netaddrp, int *lenp)
{
    switch (NetaddrFamily(netaddrp)) {
#ifdef UNIXCONN
    case AF_UNIX:
	*lenp = strlen(((struct sockaddr_un *)netaddrp)->sun_path);
        return (char *) (((struct sockaddr_un *)netaddrp)->sun_path);
#endif
#ifdef TCPCONN
    case AF_INET:
        *lenp = sizeof (struct in_addr);
        return (char *) &(((struct sockaddr_in *)netaddrp)->sin_addr);
#endif
    default:
	*lenp = 0;
	return NULL;
    }
}


/* given an XdmcpNetaddr, sets *addr to the network address used and
   sets *len to the number of bytes in addr.
   Returns the X protocol family used, e.g., FamilyInternet */

int ConvertAddr (XdmcpNetaddr saddr, int *len, char **addr)
{
    int retval;

    if (len == NULL)
        return -1;
    *addr = NetaddrAddress(saddr, len);
    switch (NetaddrFamily(saddr))
    {
#ifdef AF_UNSPEC
      case AF_UNSPEC:
	retval = FamilyLocal;
	break;
#endif
#ifdef AF_UNIX
#ifndef hpux
      case AF_UNIX:
        retval = FamilyLocal;
	break;
#endif
#endif
#ifdef TCPCONN
      case AF_INET:
        retval = FamilyInternet;
	break;
#endif
      default:
	retval = -1;
        break;
    }
    WDMDebug("ConvertAddr returning %d for family %d\n", retval,
	   NetaddrFamily(saddr));
    return retval;
}

int
addressEqual (XdmcpNetaddr a1, int len1, XdmcpNetaddr a2, int len2)
{
    int partlen1, partlen2;
    char *part1, *part2;

    if (len1 != len2)
    {
	return FALSE;
    }
    if (NetaddrFamily(a1) != NetaddrFamily(a2))
    {
	return FALSE;
    }
    part1 = NetaddrPort(a1, &partlen1);
    part2 = NetaddrPort(a2, &partlen2);
    if (partlen1 != partlen2 || memcmp(part1, part2, partlen1) != 0)
    {
	return FALSE;
    }
    part1 = NetaddrAddress(a1, &partlen1);
    part2 = NetaddrAddress(a2, &partlen2);
    if (partlen1 != partlen2 || memcmp(part1, part2, partlen1) != 0)
    {
	return FALSE;
    }
    return TRUE;
}

#endif /* XDMCP */
