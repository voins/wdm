#ifndef __WDMCONFIG_H
#define __WDMCONFIG_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(HAVE_LIBINTL_H) && defined(I18N)
#include <libintl.h>
#define _(text) gettext(text)
#else
#define _(text) text
#endif

#define N_(x) x
/* kdebase-1.0/kdm/kdm-config.h was used as a model */

/* xdm stuff which should always be defined */

#define UNIXCONN
#define TCPCONN
#define GREET_USER_STATIC

#ifdef HAVE_PAM
#   define USE_PAM
#else
#   ifdef HAVE_SHADOW
#      define USESHADOW
#   endif
#endif


/* per kde/kdm, too many systems have trouble with secure rpc */
/* disable secure rpc 'for now' */
#undef SECURE_RPC

#ifdef sun
#define SVR4 1
#endif

#endif /* __WDMCONFIG_H */
