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
 * Login.c: draw login panel and interact with user.
 */

#include <wdmconfig.h>
#include <wdmlib.h>
#include <wdmLogin.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#ifdef HAVE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif
/* temporary hack {{{ */
#include <WINGs/WINGsP.h>
/* }}} */
#include <limits.h>
#include <locale.h>
#include <time.h>

/*###################################################################*/

/** Global Variables and Constants **/

#define FOREVER 1

WMRect screen;
static int help_heigth = 140;
static int text_width = 150, text_heigth = 26;

static char displayArgDefault[] = "";
static char *displayArg = displayArgDefault;

/*###################################################################*/

static int WmDefUser = False;	/* default username */

static char *helpArg = NULL;
static char *HelpMsg =
N_("wdm is a graphical "
   "interface used to authenticate a user to "
   "the system and perform the login process.\n\n\n"
   "Enter your user name (userid) at the prompt and press "
   "<enter>.  The panel will then present a prompt to "
   "enter your password.  Enter the password and "
   "press <enter>.\n\n\n"
   "The login will then be performed and your "
   "window manager started.\n\n\n"
   "The Start WM PopUp selection specifies the parameter "
   "to pass to Xsession to start the window manager.\n\n\n"
   "NoChange will start the same window manager the user "
   "used for their last session.\n\n\n"
   "failsafe is a simple xterm session and the other "
   "listed options will start the indicated "
   "(installation specific) window manager.\n\n\n"
   "The Options PopUp selection specifies:\n\n"
   "     Login - logon to the system\n\n"
   "     Reboot - shutdown and reboot the system\n\n"
   "     Halt - shutdown the system and halt\n\n"
   "     ExitLogin - exit the display manager\n\n\n"
   "The installation may require a valid username and password "
   "or username=root and root's password to perform Reboot, "
   "Halt or Exit.\n\n\n"
   "NOTE:  "
   "ExitLogin (or, as it is sometimes refered to: exit) "
   "is intended for use primarily in wdm testing.  "
   "It will shut down the x-server but the wdm must be "
   "terminated by other means.  Starting wdm as a detached "
   "process will result that it will be very difficult "
   " to terminate.\n\n\n"
   "ExitLogin performs the same operation as ctrl-r does "
   "for xdm.\n\n\n"
   "1. The StartOver button will erase the current login "
   "Information and begin the login process again.\n\n\n"
   "2. See the man page for additional information on "
   "configuring this package.  "
   "There are numerous options for setting "
   "the background color or pixmap, the LoginPanel logo, "
   "the selection of window managers to start, and "
   "the login verification for Reboot, halt and exit.");

/*###################################################################*/

typedef struct LoginPanel
{
	WMScreen *scr;
	WMWindow *win;
	WMFrame *winF1;
	WMFrame *logoF1, *logoF2;
	WMLabel *logoL;
	WMFrame *authF;
	WMLabel *welcomeMsg1, *welcomeMsg2;
	WMTextField *entryText;
	WMLabel *entryLabel;
	WMFrame *msgF;
	WMLabel *msgL;
	int msgFlag;
	WMFrame *wmF;
	WMPopUpButton *wmBtn;
	WMFrame *exitF;
	WMPopUpButton *exitBtn;
	WMFrame *cmdF;
	WMButton *helpBtn;
	WMButton *startoverBtn;
	WMButton *goBtn;
	WMFrame *helpF;
	WMScrollView *helpSV;
	WMFrame *helpTextF;
	WMLabel *helpTextL;
	KeyCode retkey;
	KeyCode tabkey;
}
LoginPanel;

static LoginPanel *panel = NULL;

/*###################################################################*/

static int LoginSwitch = False;
static char *LoginName = NULL;
static char *LoginPswd = NULL;

static int OptionCode = 0;
static char *ExitStr[] = { N_("Login"), N_("Reboot"), N_("Halt"),
	N_("ExitLogin"), NULL
};

static char *ExitFailStr[] = { N_("Login failed"), N_("Reboot failed"),
	N_("Halt failed"), N_("ExitLogin failed"), NULL
};

static int WmOptionCode = 0;
static char WmDefault[] = "wmaker:afterstep:xsession";
static char *WmArg = WmDefault;
static char **WmStr = NULL;

static char *logoArg = NULL;
static char *bgArg = NULL;
static char *bgOption = NULL;
static int animate = False;
static int smoothScale = True;
static char *configFile = NULL;
#ifdef HAVE_XINERAMA
static int xinerama_head = 0;
#endif

static WDMLoginConfig *cfg;

static int exit_request = 0;

char *ProgName = "Login";

char *
read_help_file(int handle)
{
	char *HelpText = NULL;
	struct stat s;

	if(fstat(handle, &s) == 0)
	{
		HelpText = wmalloc(s.st_size + 1);
		if(read(handle, HelpText, s.st_size) == -1)
		{
			WDMError("%s - read_help_file(): can't read %s\n",
				ProgName, helpArg);
			wfree(HelpText);
			return NULL;
		}
		HelpText[s.st_size] = '\0';
	}
	else
		WDMError("%s - read_help_file(): can't stat %s\n",
			ProgName, helpArg);

	return HelpText;
}

char *
parse_helpArg(void)
{
	int handle;
	char *HelpText = NULL;
	char *defaultHelpText = NULL;

	/* a good default value, even in case of errors */
	defaultHelpText = wstrconcat("wdm --- " PACKAGE_VERSION "\n\n\n\n\n",
				     gettext(HelpMsg));
	HelpText = defaultHelpText;

	if(helpArg)
	{
		if((handle = open(helpArg, O_RDONLY)) == -1)
		{
			WDMError("%s - parse_helpArg(): can't open %s\n",
				ProgName, helpArg);
			return defaultHelpText;
		}

		if((HelpText = read_help_file(handle)) != NULL)
			wfree(defaultHelpText);

		close(handle);
	}

	return HelpText;
}

/*###################################################################*/

void
wAbort()			/* for WINGs compatibility */
{
	WDMPanic("%s - wAbort from WINGs\n", ProgName);
}

/*###################################################################*/

/*** pipe I/O routines ***/

/** The following code was adapted from out.c by Tom Rothamel */

/* This file is Copyright 1998 Tom Rothamel. It's under the Gnu Public	 *
 * license, see the file COPYING for details.				 */

void
writeuc(int fd, unsigned char c)
{
	write(fd, &c, sizeof(unsigned char));
}

void
writestring(int fd, char *string)
{
	int len;

	len = strlen(string);
	if(len > 255)
		len = 255;

	writeuc(fd, (unsigned char) len);
	write(fd, string, len);
}


/*** communicate authentication information ***/

static void
OutputAuth(char *user, char *pswd)
{
	writestring(3, user ? user : "");
	writestring(3, pswd ? pswd : "");

	if(OptionCode == 0)
	{
		if(WmOptionCode == 0)
			writeuc(3, 0);	/* end of data */
		else
		{
			writeuc(3, 1);
			writestring(3, WmStr[WmOptionCode]);
			writeuc(3, 0);	/* end of data */
		}
	}
	else
	{
		writeuc(3, OptionCode + 1);
		writestring(3, ExitStr[OptionCode]);
		writeuc(3, 0);	/* end of data */
	}
	return;
}

/*###################################################################*/

static void
SetupWm()
{
	int i = 0, n = 0;
	char *ptr = WmArg;

	/* count number of items, skip empty items.
	   n = number of items - 1 */
	while(*ptr)
		if(*ptr++ == ':' && *ptr != ':' && *ptr)
			++n;
	/* reserve one position fo NULL pointer, one for 'NoChange'
	   and one for 'FailSafe' */
	WmStr = (char **) malloc(sizeof(char *) * (n + 4));
	WmStr[i++] = N_("NoChange");

	if(strcasecmp(WmArg, "none") != 0)	/* we explicitly don't want any
						   choice */
	{
		ptr = WmArg;
		while(*ptr)
		{
			while(*ptr == ':')
				++ptr;
			if(!*ptr)
				break;
			WmStr[i++] = ptr;
			while(*ptr != ':' && *ptr)
				++ptr;
			if(!*ptr)
				break;
			*ptr++ = '\0';
		}
		WmStr[i++] = N_("failsafe");
	}
	WmStr[i] = NULL;
}


static void
LoginArgs(int argc, char *argv[])
{
	int c;


	while((c = getopt(argc, argv, "asb:d:h:l:uw:c:x:")) != -1)
	{
		switch (c)
		{
		case 'a':
			animate = True;
			break;
		case 's':
			smoothScale = False;
			break;
		case 'd':	/* display */
			displayArg = optarg;
			break;
		case 'h':	/* helpfile */
			helpArg = optarg;
			break;
		case 'l':	/* logo */
			logoArg = optarg;
			break;
		case 'u':	/* default user */
			WmDefUser = True;
			break;
		case 'w':	/* wm list */
			WmArg = optarg;
			break;
		case 'b':	/* background */
			bgArg = optarg;
			break;
		case 'c':	/* configfile */
			configFile = optarg;
			break;
#ifdef HAVE_XINERAMA
		case 'x':	/* xinerama head */
			xinerama_head = strtol(optarg, NULL, 0);
			break;
#endif
		default:
			WDMError("bad option: %c\n", c);
			break;
		}
	}
}

/*###################################################################*/

/* write error message to the panel */

static void
ClearMsgs(LoginPanel * panel)
{
	WMSetFrameRelief(panel->msgF, WRFlat);
	WMSetFrameTitle(panel->msgF, "");
	WMSetLabelText(panel->msgL, "");
	panel->msgFlag = False;
	XFlush(WMScreenDisplay(panel->scr));
}

static void
PrintErrMsg(LoginPanel * panel, char *msg)
{
	int i, x;
	struct timespec timeReq;

	XSynchronize(WMScreenDisplay(panel->scr), True);
	ClearMsgs(panel);
	WMSetFrameRelief(panel->msgF, WRGroove);
	WMSetFrameTitle(panel->msgF, _("ERROR"));
	WMSetLabelText(panel->msgL, msg);
	panel->msgFlag = True;
	XFlush(WMScreenDisplay(panel->scr));

	/* shake the panel like Login.app */
	if(animate)
	{
		timeReq.tv_sec = 0;
		timeReq.tv_nsec = 15;
		for(i = 0; i < 3; i++)
		{
			for(x = 2; x <= 30; x += 10)
			{
				WMMoveWidget(panel->win,
					cfg->geometry.pos.x + x,
					cfg->geometry.pos.y);
				nanosleep(&timeReq, NULL);
			}
			for(x = 30; x >= -30; x -= 10)
			{
				WMMoveWidget(panel->win,
					cfg->geometry.pos.x + x,
					cfg->geometry.pos.y);
				nanosleep(&timeReq, NULL);
			}
			for(x = -28; x <= 0; x += 10)
			{
				WMMoveWidget(panel->win,
					cfg->geometry.pos.x + x,
					cfg->geometry.pos.y);
				nanosleep(&timeReq, NULL);
			}
			XFlush(WMScreenDisplay(panel->scr));
		}
	}
	XSynchronize(WMScreenDisplay(panel->scr), False);
}

/* write info message to panel */

static void
PrintInfoMsg(LoginPanel * panel, char *msg)
{
	XSynchronize(WMScreenDisplay(panel->scr), True);
	ClearMsgs(panel);
	WMSetLabelText(panel->msgL, msg);
	XFlush(WMScreenDisplay(panel->scr));
	panel->msgFlag = True;
	XSynchronize(WMScreenDisplay(panel->scr), False);
}

/*###################################################################*/

static void
init_pwdfield(char *pwd)
{
	WMSetTextFieldText(panel->entryText, pwd);
	WMSetTextFieldSecure(panel->entryText, True);
	WMSetLabelText(panel->entryLabel, _("Password:"));
}

static void
init_namefield(char *name)
{
	WMResizeWidget(panel->entryText, text_width, text_heigth);
	WMSetTextFieldText(panel->entryText, name);
	WMSetLabelText(panel->entryLabel, _("Login name:"));
	WMSetFocusToWidget(panel->entryText);
	WMSetTextFieldSecure(panel->entryText, False);
}

static void
InitializeLoginInput(LoginPanel * panel)
{
	LoginSwitch = False;
	if(LoginName)
		wfree(LoginName);
	LoginName = NULL;
	if(LoginPswd)
		wfree(LoginPswd);
	LoginPswd = NULL;

	init_namefield("");
}

static void
PerformLogin(LoginPanel * panel, int canexit)
{
	if(LoginSwitch == False)
	{
		if(LoginName)
			wfree(LoginName);
		LoginName = WMGetTextFieldText(panel->entryText);

		if((LoginName[0] == '\0') && (WmDefUser == False))
		{
			InitializeLoginInput(panel);
			PrintErrMsg(panel, _("invalid name"));
			return;
		}

		LoginSwitch = True;

		init_pwdfield(LoginPswd);
		return;
	}
	LoginSwitch = False;
	if(LoginPswd)
		wfree(LoginPswd);
	LoginPswd = WMGetTextFieldText(panel->entryText);

	if(canexit == False)
	{
		init_namefield(LoginName);
		return;
	}

	init_namefield("");
	if(OptionCode == 0)
		PrintInfoMsg(panel, _("validating"));
	else
		PrintInfoMsg(panel, _("exiting"));

	OutputAuth(LoginName, LoginPswd);
}

/*###################################################################*/

/* Actions */


static void
goPressed(WMWidget * self, LoginPanel * panel)
{
	if(OptionCode == 0)
	{
		if(LoginSwitch == False)
		{
			PerformLogin(panel, True);
			if(LoginSwitch == False)
				return;
		}
		PerformLogin(panel, True);
		return;
	}
	if(LoginSwitch == True)
	{
		if(LoginPswd)
			wfree(LoginPswd);
		LoginPswd = WMGetTextFieldText(panel->entryText);
		WMSetTextFieldText(panel->entryText, "");
	}
	PrintInfoMsg(panel, _("exiting"));
	OutputAuth(LoginName, LoginPswd);
}

static void
startoverPressed(WMWidget * self, LoginPanel * panel)
{
	ClearMsgs(panel);
	InitializeLoginInput(panel);
}

static void
helpPressed(WMWidget * self, LoginPanel * panel)
{
	static Bool helpshown = False;
	if(!helpshown)
	{
		helpshown = True;
		WMSetButtonText(panel->helpBtn, _("Close Help"));
		WMResizeWidget(panel->win, WMWidgetWidth(panel->win),
				WMWidgetHeight(panel->win) + help_heigth);
	}
	else
	{
		helpshown = False;
		WMSetButtonText(panel->helpBtn, _("Help"));
		WMResizeWidget(panel->win, WMWidgetWidth(panel->win),
				WMWidgetHeight(panel->win) - help_heigth);
	}
}

static void
changeWm(WMWidget * self, LoginPanel * panel)
{
	WmOptionCode = WMGetPopUpButtonSelectedItem(self);
	WMSetFocusToWidget(panel->entryText);
}

static void
changeOption(WMPopUpButton * self, LoginPanel * panel)
{
	int item;

	item = WMGetPopUpButtonSelectedItem(self);
	OptionCode = item;
	WMSetFocusToWidget(panel->entryText);
}

static void
handleKeyPress(XEvent * event, void *clientData)
{
	LoginPanel *panel = (LoginPanel *) clientData;

	if(panel->msgFlag)
	{
		ClearMsgs(panel);
	}
	if(event->xkey.keycode == panel->retkey)
	{
		PerformLogin(panel, True);
	}
	else if(event->xkey.keycode == panel->tabkey)
	{
		PerformLogin(panel, False);
	}
}

/*###################################################################*/

/* create and destroy our panel */

static void
CreateLogo(LoginPanel * panel)
{
	RImage *image1, *image2;
	WMPixmap *pixmap;
	RColor gray;
	RContext *context;
	unsigned w = 200, h = 130;
	float ratio = 1.;

	panel->logoF1 = WMCreateFrame(panel->winF1);
	WMSetFrameRelief(panel->logoF1, WRSunken);
	WMSetFrameTitlePosition(panel->logoF1, WTPAtTop);
	WMMoveWidget(panel->logoF1, 15, 18);
	WMResizeWidget(panel->logoF1, 206, 136);

	panel->logoF2 = WMCreateFrame(panel->logoF1);
	WMSetFrameRelief(panel->logoF2, WRSunken);
	WMSetFrameTitlePosition(panel->logoF2, WTPAtTop);
	WMMoveWidget(panel->logoF2, 1, 1);
	WMResizeWidget(panel->logoF2, 204, 134);

	panel->logoL = WMCreateLabel(panel->logoF2);
	WMMoveWidget(panel->logoL, 2, 2);
	WMResizeWidget(panel->logoL, 200, 130);
	WMSetLabelImagePosition(panel->logoL, WIPImageOnly);

	context = WMScreenRContext(panel->scr);
	image1 = NULL;
	if(logoArg != NULL)
	{
		image1 = RLoadImage(context, logoArg, 0);
	}
	if(image1 == NULL)
	{
		RColor first, second;
		first.red = 0xae;
		first.green = 0xaa;
		first.blue = 0xc0;
		second.red = 0xae;
		second.green = 0xaa;
		second.blue = 0xae;
		image1 = RRenderGradient(200, 300, &first, &second, RDiagonalGradient);
	}
	if(image1 == NULL)
		return;

#if 0
	WDMDebug("width=%i,heigth=%i\n", image1->width, image1->height);
#endif
		if(image1->width > 200)
	{			/* try to keep the aspect ratio */
		ratio = (float) 200. / (float) image1->width;
		h = (int) ((float) image1->height * ratio);
	}
#if 0
	WDMDebug("new: ratio=%.5f,width=%i,heigth=%i\n", ratio, w, h);
#endif
		if(image1->height > 130)
	{
		if(h > 130)
		{
			ratio = (float) 130. / (float) h;
			w = (int) ((float) w * ratio);
			h = 130;
		}
	}
#if 0
	WDMDebug("new: ratio=%.5f,width=%i,heigth=%i\n", ratio, w, h);
#endif
		/* if image is too small, do not reallly resize since this looks bad */
		/* the image will be centered */
	if((image1->width < 200) && (image1->height < 130))
	{
		w = image1->width;
		h = image1->height;
	}
	/* last check in case the above logic is faulty */
	if(w > 200)
		w = 200;
	if(h > 130)
		h = 130;
#if 0
	WDMDebug("new: ratio=%.5f,width=%i,heigth=%i\n", ratio, w, h);
#endif
	if(smoothScale)
		image2 = RSmoothScaleImage(image1, w, h);
	else
		image2 = RScaleImage(image1, w, h);

	RReleaseImage(image1);
	if(image2 == NULL)
		return;
	gray.red = 0xae;
	gray.green = 0xaa;
	gray.blue = 0xae;
	RCombineImageWithColor(image2, &gray);
	pixmap = WMCreatePixmapFromRImage(panel->scr, image2, 0);
	RReleaseImage(image2);

	if(pixmap == NULL)
	{
		WDMError("unable to load pixmap\n");
		return;
	}
	WMSetLabelImage(panel->logoL, pixmap);
	WMReleasePixmap(pixmap);

}

static void
CreateAuthFrame(LoginPanel * panel)
{
	char str[128] = "?";
	WMFont *font = NULL;
	int y;

	panel->authF = WMCreateFrame(panel->winF1);
	WMSetFrameRelief(panel->authF, WRGroove);
	WMSetFrameTitlePosition(panel->authF, WTPAtTop);
	WMSetFrameTitle(panel->authF, _("Login Authentication"));
	WMMoveWidget(panel->authF, (WMWidgetWidth(panel->win) - 290), 10);
	WMResizeWidget(panel->authF, 275, 120);

	gethostname(str, 127);

	y = 20;
	panel->welcomeMsg1 = WMCreateLabel(panel->authF);
	WMResizeWidget(panel->welcomeMsg1, 255, 26);
	WMSetLabelText(panel->welcomeMsg1, _("Welcome to"));
	WMMoveWidget(panel->welcomeMsg1, 11, y);
	y += 26;
	WMSetLabelTextAlignment(panel->welcomeMsg1, WACenter);
	font = WMBoldSystemFontOfSize(panel->scr, 18);
	if(font)
	{
		WMSetLabelFont(panel->welcomeMsg1, font);
		WMReleaseFont(font);
	}

	panel->welcomeMsg2 = WMCreateLabel(panel->authF);
	WMResizeWidget(panel->welcomeMsg2, 255, 26);
	WMMoveWidget(panel->welcomeMsg2, 11, y);
	WMSetLabelText(panel->welcomeMsg2, str);
	WMSetLabelTextAlignment(panel->welcomeMsg2, WACenter);
	y = 18;
	if(strlen(str) > 20)
		y = 16;
	if(strlen(str) > 30)
		y = 14;
	if(strlen(str) > 34)
		y = 12;
	if(strlen(str) > 40)
		y = 10;
	font = WMBoldSystemFontOfSize(panel->scr, y);
	if(font)
	{
		WMSetLabelFont(panel->welcomeMsg2, font);
		WMReleaseFont(font);
	}

	y = 84;

	panel->entryLabel = WMCreateLabel(panel->authF);
	WMMoveWidget(panel->entryLabel, 10, y);
	WMResizeWidget(panel->entryLabel, 100, 26);
	font = WMBoldSystemFontOfSize(panel->scr, 14);
	if(font)
	{
		WMSetLabelFont(panel->entryLabel, font);
		WMReleaseFont(font);
	}
	WMSetLabelText(panel->entryLabel, _("Login name:"));
	WMSetLabelTextAlignment(panel->entryLabel, WARight);

	panel->entryText = WMCreateTextField(panel->authF);
	WMMoveWidget(panel->entryText, 115, y);
	WMResizeWidget(panel->entryText, text_width, text_heigth);
	WMSetTextFieldText(panel->entryText, "");
	WMSetTextFieldSecure(panel->entryText, False);
}

static void
CreateMsgsFrames(LoginPanel * panel)
{
	WMFont *font;

	panel->msgF = WMCreateFrame(panel->winF1);
	WMSetFrameRelief(panel->msgF, WRFlat);
	WMSetFrameTitlePosition(panel->msgF, WTPAtBottom);
	WMMoveWidget(panel->msgF, (WMWidgetWidth(panel->win) - 290), 136);
	WMResizeWidget(panel->msgF, 275, 40);
	WMSetFrameTitle(panel->msgF, "");

	panel->msgL = WMCreateLabel(panel->msgF);
	WMResizeWidget(panel->msgL, 260, 26);
	WMMoveWidget(panel->msgL, 5, 2);
	font = WMBoldSystemFontOfSize(panel->scr, 14);
	if(font)
	{
		WMSetLabelFont(panel->msgL, font);
		WMReleaseFont(font);
	}
	WMSetLabelText(panel->msgL, "");
	WMSetLabelTextAlignment(panel->msgL, WARight);

}

static void
CreatePopups(LoginPanel * panel)
{
	int i;

	panel->wmF = WMCreateFrame(panel->winF1);
	WMSetFrameRelief(panel->wmF, WRGroove);
	WMSetFrameTitlePosition(panel->wmF, WTPAtTop);
	WMSetFrameTitle(panel->wmF, _("Start WM"));
	WMMoveWidget(panel->wmF, 13, 178);
	WMResizeWidget(panel->wmF, 118, 45);

	panel->wmBtn = WMCreatePopUpButton(panel->wmF);
	WMMoveWidget(panel->wmBtn, 4, 15);
	WMResizeWidget(panel->wmBtn, 110, 25);
	WMSetPopUpButtonAction(panel->wmBtn, (WMAction *) changeWm, panel);
	i = 0;
	while(WmStr[i] != NULL)
	{
		WMAddPopUpButtonItem(panel->wmBtn, gettext(WmStr[i]));
		i++;
	}

	panel->exitF = WMCreateFrame(panel->winF1);
	WMSetFrameRelief(panel->exitF, WRGroove);
	WMSetFrameTitlePosition(panel->exitF, WTPAtTop);
	WMSetFrameTitle(panel->exitF, _("Options"));
	WMMoveWidget(panel->exitF, 134, 178);
	WMResizeWidget(panel->exitF, 98, 45);

	panel->exitBtn = WMCreatePopUpButton(panel->exitF);
	WMMoveWidget(panel->exitBtn, 4, 15);
	WMResizeWidget(panel->exitBtn, 90, 25);
	WMSetPopUpButtonAction(panel->exitBtn, (WMAction *) changeOption,
			       panel);
	i = 0;
	while(ExitStr[i] != NULL)
	{
		WMAddPopUpButtonItem(panel->exitBtn, gettext(ExitStr[i]));
		i++;
	}
}

static void
CreateButtons(LoginPanel * panel)
{
	int i;

	panel->cmdF = WMCreateFrame(panel->winF1);
	WMSetFrameRelief(panel->cmdF, WRFlat);
	WMSetFrameTitlePosition(panel->cmdF, WTPAtTop);
	WMMoveWidget(panel->cmdF, (WMWidgetWidth(panel->win) - 290), 185);
	WMResizeWidget(panel->cmdF, 282, 38);

	i = 3;
	panel->helpBtn = WMCreateCommandButton(panel->cmdF);
	WMSetButtonAction(panel->helpBtn, (WMAction *) helpPressed, panel);
	WMMoveWidget(panel->helpBtn, i, 8);
	WMSetButtonText(panel->helpBtn, _("Help"));
	WMResizeWidget(panel->helpBtn, 80, 25);

	i += 96;
	panel->startoverBtn = WMCreateCommandButton(panel->cmdF);
	WMSetButtonAction(panel->startoverBtn, (WMAction *) startoverPressed,
			  panel);
	WMMoveWidget(panel->startoverBtn, i, 8);
	WMSetButtonText(panel->startoverBtn, _("Start Over"));
	WMResizeWidget(panel->startoverBtn, 80, 25);

	i += 96;
	panel->goBtn = WMCreateCommandButton(panel->cmdF);
	WMSetButtonAction(panel->goBtn, (WMAction *) goPressed, panel);
	WMMoveWidget(panel->goBtn, i, 8);
	WMSetButtonText(panel->goBtn, _("Go!"));
	WMResizeWidget(panel->goBtn, 80, 25);
}

static void
CreateHelpFrames(LoginPanel * panel)
{
	int height;
	char *HelpText = NULL;

	panel->helpF = WMCreateFrame(panel->win);
	WMSetFrameRelief(panel->helpF, WRRaised);
	WMMoveWidget(panel->helpF, 0, WMWidgetHeight(panel->win));
	WMResizeWidget(panel->helpF, WMWidgetWidth(panel->win), help_heigth);

	panel->helpSV = WMCreateScrollView(panel->helpF);
	WMResizeWidget(panel->helpSV,
			(WMWidgetWidth(panel->win) - 10), (help_heigth - 10));
	WMMoveWidget(panel->helpSV, 5, 5);
	WMSetScrollViewRelief(panel->helpSV, WRSunken);
	WMSetScrollViewHasVerticalScroller(panel->helpSV, True);
	WMSetScrollViewHasHorizontalScroller(panel->helpSV, False);
	WMSetScrollViewLineScroll(panel->helpSV, 12);

	HelpText = parse_helpArg();

	panel->helpTextF = WMCreateFrame(panel->helpF);
	WMSetFrameRelief(panel->helpTextF, WRFlat);
	panel->helpTextL = WMCreateLabel(panel->helpTextF);
	WMSetLabelTextAlignment(panel->helpTextL, WALeft);

	height = W_GetTextHeight(WMDefaultSystemFont(panel->scr),
			HelpText, (WMWidgetWidth(panel->win) - 60), True) + 10;

	WMResizeWidget(panel->helpTextF,
			(WMWidgetWidth(panel->win) - 50), height);
	WMMoveWidget(panel->helpTextL, 2, 1);
	WMResizeWidget(panel->helpTextL,
			(WMWidgetWidth(panel->win) - 60), height - 5);

	WMSetLabelText(panel->helpTextL, HelpText);
	WMSetLabelWraps(panel->helpTextL, True);

	wfree(HelpText);
}

static LoginPanel *
CreateLoginPanel(WMScreen *scr, WDMLoginConfig *cfg)
{
	LoginPanel *panel;

	panel = malloc(sizeof(LoginPanel));
	if(!panel)
		return NULL;
	memset(panel, 0, sizeof(LoginPanel));
	panel->scr = scr;

	/* basic window and frames */

	panel->win = WMCreateWindow(scr, ProgName);
	WMResizeWidget(panel->win,
			cfg->geometry.size.width, cfg->geometry.size.height);

	panel->winF1 = WMCreateFrame(panel->win);
	WMResizeWidget(panel->winF1,
			cfg->geometry.size.width, cfg->geometry.size.height);
	WMSetFrameRelief(panel->winF1, WRRaised);

	CreateAuthFrame(panel);

	CreateLogo(panel);

	CreatePopups(panel);

	CreateButtons(panel);

	CreateMsgsFrames(panel);

	CreateHelpFrames(panel);

	WMRealizeWidget(panel->win);
	WMMapSubwidgets(panel->win);

	WMMapSubwidgets(panel->winF1);
	WMMapSubwidgets(panel->logoF1);
	WMMapSubwidgets(panel->logoF2);
	WMMapSubwidgets(panel->authF);
	WMMapSubwidgets(panel->wmF);
	WMMapSubwidgets(panel->exitF);
	WMMapSubwidgets(panel->cmdF);
	WMMapSubwidgets(panel->msgF);
	WMMapSubwidgets(panel->helpF);
	WMMapWidget(panel->helpF);
	WMMapSubwidgets(panel->helpSV);
	WMMapSubwidgets(panel->helpTextF);
	WMSetScrollViewContentView(panel->helpSV,
				   WMWidgetView(panel->helpTextF));

	WMSetPopUpButtonSelectedItem(panel->wmBtn, 0);
	WMSetPopUpButtonSelectedItem(panel->exitBtn, 0);

	panel->msgFlag = False;

	return panel;
}

static void
DestroyLoginPanel(LoginPanel * panel)
{
	int width, height;
	struct timespec timeReq;

	/* roll up the window before destroying it */
	if(animate)
	{
		timeReq.tv_sec = 0;
		timeReq.tv_nsec = 400;
		XSynchronize(WMScreenDisplay(panel->scr), True);	/* slow things up */
		for(width = WMWidgetWidth(panel->win) - 2,
			height = WMWidgetHeight(panel->win) - 1;
		    (height > 0 && width > 0); height -= 15, width -= 30)
		{
			WMResizeWidget(panel->win, width, height);
			nanosleep(&timeReq, NULL);
		}
		XSynchronize(WMScreenDisplay(panel->scr), False);
	}
	WMUnmapWidget(panel->win);
	WMDestroyWidget(panel->win);
	free(panel);
}

/*###################################################################*/

/** set the background **/

static int
parseBG()
{
	char *tmp;

	if(bgArg == NULL)
		return 0;
	tmp = strchr(bgArg, ':');
	if(tmp == NULL)
		return 0;
	*tmp = '\0';
	bgOption = tmp + 1;
	while(*bgOption == ' ')
		bgOption++;
	if(*bgOption == '\0')
		return 0;
	tmp = bgArg;
	while(*tmp != '\0')
	{
		*tmp = tolower(*tmp);
		tmp++;
	}
	if(strcmp(bgArg, "pixmap") == 0)
		return 1;
	if(strcmp(bgArg, "solid") == 0)
		return 2;
	if(strcmp(bgArg, "hgradient") == 0)
		return 3;
	if(strcmp(bgArg, "vgradient") == 0)
		return 4;
	if(strcmp(bgArg, "dgradient") == 0)
		return 5;
	return 0;
}

static RImage *
loadBGpixmap(RContext * rcontext)
{
	RImage *image, *tmp;

	image = RLoadImage(rcontext, bgOption, 0);
	if(image == NULL)
	{
		WDMError("%s could not load bg image %s\n",
			ProgName, bgOption);
		return NULL;
	}
	tmp = RScaleImage(image, screen.size.width, screen.size.height);
	if(tmp == NULL)
	{
		WDMError("%s could not resize bg image %s\n",
			ProgName, bgOption);
		RReleaseImage(image);
		return NULL;
	}
	RReleaseImage(image);
	return tmp;
}

static RColor **
allocmem(int num)
{
	RColor **colors = NULL;
	int i;

	colors = malloc(sizeof(RColor *) * (num + 1));
	for(i = 0; i < num; i++)
	{
		colors[i] = malloc(sizeof(RColor));
	}
	colors[i] = NULL;
	return colors;
}

static void
freemem(int num, RColor ** colors)
{
	int i;

	for(i = 0; i < num; i++)
	{
		free(colors[i]);
	}
	free(colors);
}

static RImage *
createBGcolor(WMScreen * scr, RContext * rcontext, char *str, int style)
{
	RImage *image;
	RColor **colors = NULL;
	XColor color;
	int num_colors = 0;
	int i;
	char *tmp, *colorstr;

	colorstr = str;
	while(*colorstr)
	{
		num_colors++;
		tmp = strchr(colorstr, ',');
		if(tmp == NULL)
			colorstr = str + strlen(str);
		else
			colorstr = tmp + 1;
	}
	if(num_colors == 0)
		return NULL;
	colors = allocmem(num_colors);
	tmp = str;
	for(i = 0; i < num_colors; i++)
	{
		colorstr = tmp;
		tmp = strchr(tmp, ',');
		if(tmp != NULL)
		{
			*tmp = '\0';
			tmp++;
		}
		else
			tmp = colorstr + strlen(colorstr);
		if(!XParseColor
		   (WMScreenDisplay(scr), rcontext->cmap, colorstr, &color))
		{
			WDMError("could not parse color \"%s\"\n",
				colorstr);
			freemem(num_colors, colors);
			return NULL;
		}
		colors[i]->red = color.red >> 8;
		colors[i]->green = color.green >> 8;
		colors[i]->blue = color.blue >> 8;
	}
	image = RRenderMultiGradient(screen.size.width, screen.size.height,
				     colors, style);
	freemem(num_colors, colors);
	return image;
}

static void
setBG(WMScreen * scr)
{
	Window root_window;
	int cpc = 4, render_mode = RBestMatchRendering, default_depth = 8;
	RContextAttributes rattr;
	RContext *rcontext;
	RImage *image;
	XColor defcolor;
	Pixmap pixmap;

	/* if not specified or none, then skip setting background */
	/* user can still set background via other means */
	if(bgArg == NULL)
		return;
	if(strcasecmp(bgArg, "none") == 0)
		return;

	/* use of scr->rootWin is temporary hack */
	root_window = scr->rootWin;
	default_depth = WMScreenDepth(scr);
	if(default_depth <= 8)
		render_mode = RDitheredRendering;
	rattr.flags = RC_RenderMode | RC_ColorsPerChannel;
	rattr.render_mode = render_mode;
	rattr.colors_per_channel = cpc;
	/* use of scr->screen is temporary hack */
	rcontext = RCreateContext(WMScreenDisplay(scr), scr->screen, &rattr);
	if(rcontext == NULL)
	{
		WDMError("%s could not initialize "
			"graphics library context: %s\n",
			ProgName, RMessageForError(RErrorCode));
		return;
	}

	defcolor.pixel = 0L;	/* default=black */

	switch (parseBG())
	{
	case 1:
		image = loadBGpixmap(rcontext);
		break;
	case 2:
		image = createBGcolor(scr, rcontext, bgOption, RGRD_HORIZONTAL);
		break;
	case 3:
		image = createBGcolor(scr, rcontext, bgOption, RGRD_HORIZONTAL);
		break;
	case 4:
		image = createBGcolor(scr, rcontext, bgOption, RGRD_VERTICAL);
		break;
	case 5:
		image = createBGcolor(scr, rcontext, bgOption, RGRD_DIAGONAL);
		break;
	default:
		image = NULL;
		break;
	}
	if(image == NULL)
	{
		XSetWindowBackground(WMScreenDisplay(scr), root_window, 0L);
		XClearWindow(WMScreenDisplay(scr), root_window);
		XFlush(WMScreenDisplay(scr));
		return;
	}
	RConvertImage(rcontext, image, &pixmap);
	RReleaseImage(image);
	XSetWindowBackgroundPixmap(WMScreenDisplay(scr), root_window, pixmap);
	XClearWindow(WMScreenDisplay(scr), root_window);
	XFlush(WMScreenDisplay(scr));
}

/*###################################################################*/

/* signal processing */

static void
SignalUsr1(int ignored)		/* oops, an error */
{
	InitializeLoginInput(panel);
	PrintErrMsg(panel, gettext(ExitFailStr[OptionCode]));
	signal(SIGUSR1, SignalUsr1);
}

static void
SignalTerm(int ignored)		/* all done */
{
	exit_request = 1;	/* corrects some hanging problems, thanks to A. Kabaev */
}

/*###################################################################*/

/*  M A I N  */

int
main(int argc, char **argv)
{
	WMScreen *scr;
	int xine_count;

#ifdef HAVE_XINERAMA
	XineramaScreenInfo *xine;
#endif

	ProgName = argv[0];

	setlocale(LC_ALL, "");

#ifdef I18N
	if(getenv("NLSPATH"))
		bindtextdomain("wdm", getenv("NLSPATH"));
	else
		bindtextdomain("wdm", NLSDIR);
	textdomain("wdm");
#endif

	animate = False;
	LoginArgs(argc, argv);	/* process our args */

	cfg = LoadConfiguration(configFile);	/* load configs */
	if(cfg)
	{
		printf("geometry: %ix%i+%i+%i\n",
				cfg->geometry.size.width,
				cfg->geometry.size.height,
				cfg->geometry.pos.x,
				cfg->geometry.pos.y);
	}

	SetupWm();		/* and init the startup list */

	WMInitializeApplication(ProgName, &argc, argv);
	scr = WMOpenScreen(displayArg);
	if(!scr)
	{
		WDMPanic("could not initialize Screen\n");
		exit(2);
	}

#ifdef USE_AA
	if(cfg->multibyte)
		scr->useMultiByte = True;

	if(cfg->aaenabled)
	{
		scr->antialiasedText = True;
		scr->normalFont = WMSystemFontOfSize(scr,
				WINGsConfiguration.defaultFontSize);

		scr->boldFont = WMBoldSystemFontOfSize(scr, 
				WINGsConfiguration.defaultFontSize);

		if(!scr->boldFont)
			scr->boldFont = scr->normalFont;

		if(!scr->normalFont)
		{
			WDMError("could not load any fonts.");
			exit(2);
		}
	}
#endif
	if(cfg->animations)
		animate = True;

	screen.pos.x = 0;
	screen.pos.y = 0;
	screen.size.width = WMScreenWidth(scr);
	screen.size.height = WMScreenHeight(scr);
#ifdef HAVE_XINERAMA
	if(XineramaIsActive(WMScreenDisplay(scr)))
	{
		xine = XineramaQueryScreens(WMScreenDisplay(scr), &xine_count);

		if(xine != NULL)
		{
			if(xinerama_head < xine_count)
			{
				screen.pos.x = xine[xinerama_head].x_org;
				screen.pos.y = xine[xinerama_head].y_org;
				screen.size.width = xine[xinerama_head].width;
				screen.size.height = xine[xinerama_head].height;
			}
		}
	}
#endif

	if(cfg->geometry.pos.x == INT_MIN || cfg->geometry.pos.y == INT_MIN)
	{
		cfg->geometry.pos.x = screen.pos.x +
			(screen.size.width - cfg->geometry.size.width)/2;
		cfg->geometry.pos.y = screen.pos.y +
			(screen.size.height - cfg->geometry.size.height)/2;
	}

	XSynchronize(WMScreenDisplay(scr), False);

	/* use of scr->rootWin is temporary hack */
	XWarpPointer(WMScreenDisplay(scr), None,
		     scr->rootWin,
		     0, 0, 0, 0,
		     (cfg->geometry.pos.x + (cfg->geometry.size.width - 10)),
		     (cfg->geometry.pos.y + (cfg->geometry.size.height - 10)));
	/* use of scr->rootWin is temporary hack */
	XDefineCursor(WMScreenDisplay(scr),
		      scr->rootWin,
		      XCreateFontCursor(WMScreenDisplay(scr),
					XC_top_left_arrow));

	setBG(scr);


	panel = CreateLoginPanel(scr, cfg);
	WMSetWindowTitle(panel->win, ProgName);
	/* the following Resize and the one following the Move fake out WINGs */
	/* so that the move is not visible */
	WMResizeWidget(panel->win, 1, 1);
	WMMapWidget(panel->win);
	WMSetWindowTitle(panel->win, ProgName);
	WMMoveWidget(panel->win, cfg->geometry.pos.x, cfg->geometry.pos.y);
	WMResizeWidget(panel->win, cfg->geometry.size.width, cfg->geometry.size.height);
	WMSetFocusToWidget(panel->entryText);
	XSetInputFocus(WMScreenDisplay(scr), WMWidgetXID(panel->win),
		       RevertToParent, CurrentTime);
	panel->retkey = XKeysymToKeycode(WMScreenDisplay(scr), XK_Return);
	panel->tabkey = XKeysymToKeycode(WMScreenDisplay(scr), XK_Tab);

	WMCreateEventHandler(WMWidgetView(panel->entryText), KeyPressMask,
			     handleKeyPress, panel);

	exit_request = 0;
	signal(SIGUSR1, SignalUsr1);
	signal(SIGTERM, SignalTerm);
	signal(SIGINT, SignalTerm);
	signal(SIGPIPE, SIG_DFL);

	while(!exit_request)
	{
		XEvent event;
		WMNextEvent(WMScreenDisplay(scr), &event);
		WMHandleEvent(&event);
	}
	DestroyLoginPanel(panel);

	return 0;
}
