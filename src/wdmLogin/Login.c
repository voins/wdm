/*###################################################################*/
/*##			       wdmLogin				   ##*/
/*##								   ##*/
/*##	This software is Copyright (C) 1998 by Gene Czarcinski.	   ##*/
/*## This software falls under the GNU Public License. Please read ##*/
/*##		  the COPYING file for more information		   ##*/
/*###################################################################*/


#include <wdmconfig.h>
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
#include <WINGs/WINGs.h>
#include <WINGs/WUtil.h>
#include <limits.h>

#if (WINGS_H_VERSION == 980901)
void WMSetScrollViewLineScroll(WMScrollView *sPtr, int amount);
void WMSetScrollViewPageScroll(WMScrollView *sPtr, int amount);
#endif

#include <pixmaps/gnuLogo.xpm>

/*###################################################################*/

/** Global Variables and Constants **/

#define FOREVER 1

#define P_WIDTH 530
#define P_HEIGTH 240

/* 128 is the max for MD5 passwords; is there a constant? */
#define PASS_LEN 128+1

#ifndef _POSIX_LOGIN_NAME_MAX
#ifdef LOGNAME_MAX
#define _POSIX_LOGIN_NAME_MAX LOGNAME_MAX
#else
#define _POSIX_LOGIN_NAME_MAX 9
#endif
#endif
#define LOGNAME_LEN _POSIX_LOGIN_NAME_MAX+1

Display *dpy = NULL;
static int	screen_number = 0;
static int	screen_width = 0,	screen_heigth = 0;
static int	panel_width = P_WIDTH,	panel_heigth=P_HEIGTH;
static int	help_heigth = 140;
static int	panel_X = 0,		panel_Y = 0;
static int	text_width = 150,	text_heigth=26;

static int exit_request = 0;

static char  displayArgDefault[] = "";
static char *displayArg = displayArgDefault;

/*###################################################################*/

static int WmDefUser = False;	       /* default username */

static char *helpArg = NULL;
static char *HelpFile = NULL;
static char  *HelpMsg =
		PACKAGE_NAME
		" --- Version "
		PACKAGE_VERSION
		"\n\n\n\n\n"
		PACKAGE_NAME
		" is a graphical "
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
		"the login verification for Reboot, halt and exit.";

/*###################################################################*/

typedef struct LoginPanel {
	WMScreen *scr;
	WMWindow *win;
	WMFrame	 *winF1;
	WMFrame	 *logoF1, *logoF2;
	WMLabel	 *logoL;
	WMFrame	 *authF;
	WMLabel	 *welcomeMsg1, *welcomeMsg2;
	WMTextField *entryText;
	WMLabel	 *entryLabel;
	WMFrame	 *msgF;
	WMLabel	 *msgL;
	int	  msgFlag;
	WMFrame	 *wmF;
	WMPopUpButton *wmBtn;
	WMFrame	 *exitF;
	WMPopUpButton *exitBtn;
	WMFrame	 *cmdF;
	WMButton *helpBtn;
	WMButton *startoverBtn;
	WMButton *goBtn;
	WMFrame	 *helpF;
	WMScrollView *helpSV;
	WMFrame	 *helpTextF;
	WMLabel	 *helpTextL;
	KeyCode	  retkey;
	KeyCode	  tabkey;
} LoginPanel;

static LoginPanel *panel = NULL;

/*###################################################################*/

static int   LoginSwitch   = False;
static char  LoginName[LOGNAME_LEN] = "";
static char  LoginPswd[PASS_LEN] = "";

static char *Cname  = "Login Name:";
static char *Cpswd  = "Password:";

static int   OptionCode	   = 0;
static char  OptionStr[32] = "Login";
static char  ExitLogin[]   = "Login";
static char  ExitReboot[]  = "Reboot";
static char  ExitHalt[]	   = "Halt";
static char  ExitExit[]	   = "ExitLogin";
static char *ExitStr[5]	   = {ExitLogin,ExitReboot,ExitHalt,ExitExit,
			      NULL};

static int   WmOptionCode  = 0;
static char  WmOption[256] = "NoChange";
static char  WmNoChange[]  = "NoChange";
static char  WmFailSafe[]  = "failsafe";
static char  WmDefault[]   = "wmaker:afterstep:xsession";
static char *WmArg	   = NULL;
static char *WmStr[16]	   = {WmNoChange,NULL,NULL,NULL,NULL};

static char *logoArg	   = NULL;
static char *bgArg	   = NULL;
static char *bgOption	   = NULL;
static int   animate = False;

char *ProgName= "Login";

void parse_helpArg(void)
{
	int handle;
	struct stat s;

	HelpFile = HelpMsg;	/* a good default value, even in case of errors */
	if (helpArg != NULL)
	{
		handle = open(helpArg, O_RDONLY);
		if (handle != -1)
		{
			if (! fstat(handle, &s))
			{
				HelpFile = (char *)malloc((sizeof(char) * s.st_size) + 2);
				if (HelpFile != NULL)
				{
					read(handle, HelpFile, s.st_size);
					HelpFile[s.st_size] = '\0';
				}
				else
					fprintf(stderr, "%s - parse_helpArg(): malloc failed\n", ProgName);
			}
			else
				fprintf(stderr, "%s - parse_helpArg(): can't fstat %s\n", ProgName, helpArg);
			close(handle);
		}
		else
			fprintf(stderr, "%s - parse_helpArg(): can't open %s\n", ProgName, helpArg);
	}
}

int countlines(char *str)
{
	int nbl = 0;

	while (*str)
	{
		if (*str == '\n')
			nbl++;
		str++;
	}
	return(nbl + 1); /* +1: nicer */
}

/*###################################################################*/

void wAbort()		/* for WINGs compatibility */
{
    fprintf(stderr,"%s - wAbort from WINGs\n",ProgName);
    exit(1);
}

/*###################################################################*/

/*** pipe I/O routines ***/

/** The following code was adapted from out.c by Tom Rothamel */

/* This file is Copyright 1998 Tom Rothamel. It's under the Gnu Public	 *
 * license, see the file COPYING for details.				 */

void writeuc(int fd, unsigned char c)
{
	write(fd, &c, sizeof(unsigned char));
}

void writestring(int fd, char *string)
{
	int len;

	len = strlen(string);
	if (len > 255) len = 255;

	writeuc(fd, (unsigned char) len);
	write(fd, string, len);
}


/*** communicate authentication information ***/

static void OutputAuth(char *user, char *pswd)
{
	writestring(3, user);
	writestring(3, pswd);

    if (OptionCode==0) {
	if (WmOptionCode==0)
	    writeuc(3,0); /* end of data */
	else {
	    writeuc(3,1);
	    writestring(3,WmStr[WmOptionCode]);
	    writeuc(3,0); /* end of data */
	}
    }
    else {
	writeuc(3,OptionCode+1);
	writestring(3,ExitStr[OptionCode]);
	writeuc(3,0); /* end of data */
    }
    return;
}

/*###################################################################*/

static void SetupWm()
{
    char *p1, *p2;
    int i;

    if (WmArg!=NULL)
    {
	p1=WmArg;
	if (! strcasecmp(WmArg, "none"))
		return; /* we explicitly don't want any choice */
    }
    else
	p1=WmDefault;
    i=1;
    while ((i<14) && (*p1!='\0')) {
	while (*p1 == ':')
	   p1++;
	if (*p1 == '\0')
	   break;
	WmStr[i] = p1;
	p2 = strchr(p1,':');
	if (p2==NULL)
	   break;
	*p2 = '\0';
	p1 = p2+1;
	i++;
    }
    if (*p1!='\0') {
	WmStr[i] = p1;
	i++;
    }
    WmStr[i] = WmFailSafe;
    WmStr[i+1] = NULL;
}


static void LoginArgs(int argc, char *argv[])
{
    char *tmp;
    int c;


    while(1) {
	c = getopt(argc, argv, "ab:d:h:l:uw:");
	if (c == -1)
	    break;
	switch (c) {
	    case 'a':
		animate = True;
	    break;
	    case 'd':				/* display */
		tmp = strchr(optarg,'=');
		if (tmp==NULL)
		    tmp=optarg;
		else
		    tmp++;
		displayArg = malloc(sizeof(char)*strlen(optarg)+2);
		strcpy(displayArg,tmp);
	    break;
	    case 'h':				/* helpfile */
		tmp = strchr(optarg,'=');
		if (tmp==NULL)
		    tmp=optarg;
		else
		    tmp++;
		helpArg = malloc(sizeof(char)*strlen(optarg)+2);
		strcpy(helpArg,tmp);
	    break;
	    case 'l':				/* logo */
		tmp = strchr(optarg,'=');
		if (tmp==NULL)
		    tmp=optarg;
		else
		    tmp++;
		logoArg = malloc(sizeof(char)*strlen(optarg)+2);
		strcpy(logoArg,tmp);
	    break;
	    case 'u':				/* default user */
		WmDefUser = True;
	    break;
	    case 'w':				/* wm list */
		tmp = strchr(optarg,'=');
		if (tmp==NULL)
		    tmp=optarg;
		else
		    tmp++;
		WmArg = malloc(sizeof(char)*strlen(optarg)+2);
		strcpy(WmArg,tmp);
	    break;
	    case 'b':				/* background */
		tmp = strchr(optarg,'=');
		if (tmp==NULL)
		    tmp=optarg;
		else
		    tmp++;
		bgArg = malloc(sizeof(char)*strlen(optarg)+2);
		strcpy(bgArg,tmp);
	    break;
	    default:
		fprintf(stderr,"bad option: %c\n",c);
	    break;
	}
    }
}

/*###################################################################*/

/* write error message to the panel */

static void ClearMsgs(LoginPanel *panel)
{
    WMSetFrameRelief(panel->msgF,WRFlat);
    WMSetFrameTitle(panel->msgF,"");
    WMSetLabelText(panel->msgL,"");
    panel->msgFlag = False;
    XFlush(dpy);
}

static void PrintErrMsg(LoginPanel *panel, char* msg)
{
    int i,x;

    XSynchronize(dpy,True);
    ClearMsgs(panel);
    WMSetFrameRelief(panel->msgF,WRGroove);
    WMSetFrameTitle(panel->msgF,"ERROR");
    WMSetLabelText(panel->msgL,msg);
    panel->msgFlag = True;
    XFlush(dpy);

    /* shake the panel like Login.app */
    if (animate) {
	for (i=0;i<3;i++) {
	    for (x=2;x<=20;x+=2)
		WMMoveWidget(panel->win, panel_X+x, panel_Y);
	    for (x=20;x>=-20;x-=2)
		WMMoveWidget(panel->win, panel_X+x, panel_Y);
	    for (x=-18;x<=0;x+=2)
		WMMoveWidget(panel->win, panel_X+x, panel_Y);
	    XFlush(dpy);
	}
    }
    XSynchronize(dpy,False);
}

/* write info message to panel */

static void PrintInfoMsg(LoginPanel *panel, char *msg)
{
    XSynchronize(dpy,True);
    ClearMsgs(panel);
    WMSetLabelText(panel->msgL,msg);
    XFlush(dpy);
    panel->msgFlag = True;
    XSynchronize(dpy,False);
}

/*###################################################################*/

static void init_pwdfield(char *pwd)
{
	WMSetTextFieldText(panel->entryText, pwd);
#if (WINGS_H_VERSION > 980722)
	WMSetTextFieldSecure(panel->entryText, True);
#else
	WMResizeWidget(panel->entryText, text_width, 4); /* make invisible */
#endif
	WMSetLabelText(panel->entryLabel,Cpswd);
}

static void init_namefield(char *name)
{
    WMResizeWidget(panel->entryText, text_width, text_heigth);
    WMSetTextFieldText(panel->entryText,name);
    WMSetLabelText(panel->entryLabel,Cname);
    WMSetFocusToWidget(panel->entryText);
#if (WINGS_H_VERSION > 980722)
	WMSetTextFieldSecure(panel->entryText, False);
#endif
}

static void InitializeLoginInput(LoginPanel *panel)
{
    LoginSwitch = False;
    LoginName[0] = '\0';
    LoginPswd[0] = '\0';

    init_namefield("");
}

static void PerformLogin(LoginPanel *panel, int canexit)
{
    char *tmp;

    tmp = WMGetTextFieldText(panel->entryText);
    if (LoginSwitch == False) {
	strncpy(LoginName,tmp,LOGNAME_LEN);
	if ((LoginName[0]=='\0') && (WmDefUser == False)) {
	    InitializeLoginInput(panel);
	    PrintErrMsg(panel,"invalid name");
	    return;
	}
	LoginSwitch = True;

	init_pwdfield(LoginPswd);
	return;
    }
    LoginSwitch = False;
    strncpy(LoginPswd,tmp,PASS_LEN);

    if (canexit == False)
    {
	init_namefield(LoginName);
	return;
    }

    init_namefield("");
    if (OptionCode==0)
	PrintInfoMsg(panel,"validating");
    else
	PrintInfoMsg(panel,"exiting");

    OutputAuth(LoginName, LoginPswd);
}

/*###################################################################*/

/* Actions */


static void goPressed(WMWidget *self, LoginPanel *panel)
{
    char *tmp;

    if (OptionCode==0) {
	if (LoginSwitch == False) {
	    PerformLogin(panel, True);
	    if (LoginSwitch == False)
		return;
	}
	PerformLogin(panel, True);
	return;
    }
    if (LoginSwitch == True) {
	tmp = WMGetTextFieldText(panel->entryText);
	WMSetTextFieldText(panel->entryText,"");
	strncpy(LoginPswd,tmp,PASS_LEN);
    }
    PrintInfoMsg(panel,"exiting");
    OutputAuth(LoginName,LoginPswd);
}

static void startoverPressed(WMWidget *self, LoginPanel *panel)
{
    ClearMsgs(panel);
    InitializeLoginInput(panel);
}

static void helpPressed(WMWidget *self, LoginPanel *panel)
{
    if (panel_heigth == P_HEIGTH) {
	panel_heigth = P_HEIGTH + help_heigth;
	WMSetButtonText(panel->helpBtn, "Close Help");
	WMResizeWidget(panel->win, panel_width, panel_heigth);
    }
    else {
	panel_heigth = P_HEIGTH;
	WMSetButtonText(panel->helpBtn, "Help");
	WMResizeWidget(panel->win, panel_width, panel_heigth);
    }
}

static void changeWm(WMWidget *self, LoginPanel *panel)
{
    WmOptionCode = WMGetPopUpButtonSelectedItem(self);
    strncpy(WmOption,WmStr[WmOptionCode],255);
    WmOption[255] = '\0';
    WMSetFocusToWidget(panel->entryText);
}

static void changeOption(WMPopUpButton *self, LoginPanel *panel)
{
    int item;

    item = WMGetPopUpButtonSelectedItem(self);
    strcpy(OptionStr,ExitStr[item]);
    OptionCode = item;
    WMSetFocusToWidget(panel->entryText);
}

static void handleKeyPress(XEvent *event, void *clientData)
{
    LoginPanel *panel = (LoginPanel*)clientData;

    if (panel->msgFlag) {
	ClearMsgs(panel);
    }
    if (event->xkey.keycode == panel->retkey) {
	PerformLogin(panel, True);
    }
    else if (event->xkey.keycode == panel->tabkey) {
	PerformLogin(panel, False);
    }
}

/*###################################################################*/

/* create and destroy our panel */

static void CreateLogo(LoginPanel *panel)
{
    RImage *image1, *image2;
    WMPixmap *pixmap;
    RColor gray;
    RContext *context;
    unsigned w=200, h=130;
    float ratio=1.;

    panel->logoF1 = WMCreateFrame(panel->winF1);
    WMSetFrameRelief(panel->logoF1,WRSunken);
    WMSetFrameTitlePosition(panel->logoF1,WTPAtTop);
    WMMoveWidget(panel->logoF1, 15, 18);
    WMResizeWidget(panel->logoF1, 206, 136);

    panel->logoF2 = WMCreateFrame(panel->logoF1);
    WMSetFrameRelief(panel->logoF2,WRSunken);
    WMSetFrameTitlePosition(panel->logoF2,WTPAtTop);
    WMMoveWidget(panel->logoF2, 1, 1);
    WMResizeWidget(panel->logoF2, 204, 134);

    panel->logoL = WMCreateLabel(panel->logoF2);
    WMMoveWidget(panel->logoL, 2, 2);
    WMResizeWidget(panel->logoL, 200, 130);
    WMSetLabelImagePosition(panel->logoL,WIPImageOnly);

    context = WMScreenRContext(panel->scr);
    image1 = NULL;
    if (logoArg!=NULL) {
	image1 = RLoadImage(context, logoArg, 0);
    }
    if (image1==NULL)
	image1 = RGetImageFromXPMData(context, gnuLogo_xpm);
    if (image1==NULL)
	return;

#if 0
    fprintf(stderr,"width=%i,heigth=%i\n",image1->width,image1->height);/*DEBUG*/
#endif
    if (image1->width > 200) {		/* try to keep the aspect ratio */
	ratio = (float)200. / (float)image1->width;
	h = (int) ((float)image1->height * ratio);
    }
#if 0
    fprintf(stderr,"new: ratio=%.5f,width=%i,heigth=%i\n",ratio,w,h);/*DEBUG*/
#endif
    if (image1->height > 130) {
	if (h > 130) {
	    ratio = (float) 130. / (float)h;
	    w = (int) ((float)w * ratio);
	    h = 130;
	}
    }
#if 0
    fprintf(stderr,"new: ratio=%.5f,width=%i,heigth=%i\n",ratio,w,h);/*DEBUG*/
#endif
    /* if image is too small, do not reallly resize since this looks bad */
    /* the image will be centered */
    if ((image1->width<200) && (image1->height<130)) {
	w = image1->width;
	h = image1->height;
    }
    /* last check in case the above logic is faulty */
    if (w > 200) w = 200;
    if (h > 130) h = 130;
#if 0
    fprintf(stderr,"new: ratio=%.5f,width=%i,heigth=%i\n",ratio,w,h);/*DEBUG*/
#endif
    image2 = RScaleImage(image1, w, h);
    RReleaseImage(image1);
    if (image2==NULL)
	return;
    gray.red = 0xae;
    gray.green = 0xaa;
    gray.blue = 0xae;
    RCombineImageWithColor(image2,&gray);
    pixmap = WMCreatePixmapFromRImage(panel->scr, image2, 0);
    RReleaseImage(image2);

    if (pixmap==NULL) {
	fprintf(stderr,"unable to load pixmap\n");
	return;
    }
    WMSetLabelImage(panel->logoL, pixmap);
    WMReleasePixmap(pixmap);

}

static void CreateAuthFrame(LoginPanel *panel)
{
    char str[128] = "?";
    WMFont *font=NULL;
    int y;

    panel->authF = WMCreateFrame(panel->winF1);
    WMSetFrameRelief(panel->authF,WRGroove);
    WMSetFrameTitlePosition(panel->authF,WTPAtTop);
    WMSetFrameTitle(panel->authF,"Login Authentication");
    WMMoveWidget(panel->authF, (panel_width - 290), 10);
    WMResizeWidget(panel->authF, 275, 120);

    gethostname(str,127);

    y=20;
    panel->welcomeMsg1 = WMCreateLabel(panel->authF);
    WMResizeWidget(panel->welcomeMsg1, 255, 26);
    WMSetLabelText(panel->welcomeMsg1, "Welcome to");
    WMMoveWidget(panel->welcomeMsg1, 11, y);  y += 26;
    WMSetLabelTextAlignment(panel->welcomeMsg1,WACenter);
    font = WMBoldSystemFontOfSize(panel->scr,18);
    if (font) {
	WMSetLabelFont(panel->welcomeMsg1,font);
	WMReleaseFont(font);
    }

    panel->welcomeMsg2 = WMCreateLabel(panel->authF);
    WMResizeWidget(panel->welcomeMsg2, 255, 26);
    WMMoveWidget(panel->welcomeMsg2, 11, y);
    WMSetLabelText(panel->welcomeMsg2, str);
    WMSetLabelTextAlignment(panel->welcomeMsg2,WACenter);
    y=18;
    if (strlen(str)>20)
	y=16;
    if (strlen(str)>30)
	y=14;
    if (strlen(str)>34)
	y=12;
    if (strlen(str)>40)
	y=10;
    font = WMBoldSystemFontOfSize(panel->scr,y);
    if (font) {
	WMSetLabelFont(panel->welcomeMsg2,font);
	WMReleaseFont(font);
    }

    y = 84;

    panel->entryLabel = WMCreateLabel(panel->authF);
    WMMoveWidget(panel->entryLabel, 10, y);
    WMResizeWidget(panel->entryLabel, 100, 26);
    font = WMBoldSystemFontOfSize(panel->scr,14);
    if (font) {
	WMSetLabelFont(panel->entryLabel,font);
	WMReleaseFont(font);
    }
    WMSetLabelText(panel->entryLabel,Cname);
    WMSetLabelTextAlignment(panel->entryLabel,WARight);

    panel->entryText = WMCreateTextField(panel->authF);
    WMMoveWidget(panel->entryText, 115, y);
    WMResizeWidget(panel->entryText, text_width, text_heigth);
    WMSetTextFieldText(panel->entryText,"");
#if (WINGS_H_VERSION > 980722)
	WMSetTextFieldSecure(panel->entryText, False);
#endif
}

static void CreateMsgsFrames(LoginPanel *panel)
{
    WMFont *font;

    panel->msgF = WMCreateFrame(panel->winF1);
    WMSetFrameRelief(panel->msgF,WRFlat);
    WMSetFrameTitlePosition(panel->msgF,WTPAtBottom);
    WMMoveWidget(panel->msgF, (panel_width - 290), 136);
    WMResizeWidget(panel->msgF, 275, 40);
    WMSetFrameTitle(panel->msgF,"");

    panel->msgL = WMCreateLabel(panel->msgF);
    WMResizeWidget(panel->msgL, 260, 26);
    WMMoveWidget(panel->msgL, 5, 2);
    font = WMBoldSystemFontOfSize(panel->scr,14);
    if (font) {
	WMSetLabelFont(panel->msgL,font);
	WMReleaseFont(font);
    }
    WMSetLabelText(panel->msgL,"");
    WMSetLabelTextAlignment(panel->msgL,WARight);

}

static void CreatePopups(LoginPanel *panel)
{
    int i;
    panel->wmF = WMCreateFrame(panel->winF1);
    WMSetFrameRelief(panel->wmF,WRGroove);
    WMSetFrameTitlePosition(panel->wmF,WTPAtTop);
    WMSetFrameTitle(panel->wmF,"Start WM");
    WMMoveWidget(panel->wmF, 13, 178);
    WMResizeWidget(panel->wmF, 118, 45);

    panel->wmBtn = WMCreatePopUpButton(panel->wmF);
    WMMoveWidget(panel->wmBtn, 4, 15);
    WMResizeWidget(panel->wmBtn, 110, 25);
    WMSetPopUpButtonAction(panel->wmBtn, (WMAction*)changeWm, panel);
    i=0;
    while (WmStr[i]!=NULL) {
	WMAddPopUpButtonItem(panel->wmBtn, WmStr[i]);
	i++;
    }

    panel->exitF = WMCreateFrame(panel->winF1);
    WMSetFrameRelief(panel->exitF,WRGroove);
    WMSetFrameTitlePosition(panel->exitF,WTPAtTop);
    WMSetFrameTitle(panel->exitF,"Options");
    WMMoveWidget(panel->exitF, 134, 178);
    WMResizeWidget(panel->exitF, 88, 45);

    panel->exitBtn = WMCreatePopUpButton(panel->exitF);
    WMMoveWidget(panel->exitBtn, 4, 15);
    WMResizeWidget(panel->exitBtn, 80, 25);
    WMSetPopUpButtonAction(panel->exitBtn, (WMAction*)changeOption, panel);
    i=0;
    while (ExitStr[i]!=NULL) {
	WMAddPopUpButtonItem(panel->exitBtn, ExitStr[i]);
	i++;
    }
}

static void CreateButtons(LoginPanel *panel)
{
    int i;
    panel->cmdF = WMCreateFrame(panel->winF1);
    WMSetFrameRelief(panel->cmdF,WRFlat);
    WMSetFrameTitlePosition(panel->cmdF,WTPAtTop);
    WMMoveWidget(panel->cmdF, (panel_width - 290), 185);
    WMResizeWidget(panel->cmdF, 282, 38);

    i = 3;
    panel->helpBtn = WMCreateCommandButton(panel->cmdF);
    WMSetButtonAction(panel->helpBtn, (WMAction*)helpPressed, panel);
    WMMoveWidget(panel->helpBtn, i, 8);
    WMSetButtonText(panel->helpBtn, "Help");
    WMResizeWidget(panel->helpBtn, 80, 25);

    i += 96;
    panel->startoverBtn = WMCreateCommandButton(panel->cmdF);
    WMSetButtonAction(panel->startoverBtn, (WMAction*)startoverPressed, panel);
    WMMoveWidget(panel->startoverBtn, i, 8);
    WMSetButtonText(panel->startoverBtn, "Start Over");
    WMResizeWidget(panel->startoverBtn, 80, 25);

    i += 96;
    panel->goBtn = WMCreateCommandButton(panel->cmdF);
    WMSetButtonAction(panel->goBtn, (WMAction*)goPressed, panel);
    WMMoveWidget(panel->goBtn, i, 8);
    WMSetButtonText(panel->goBtn, "Go!");
    WMResizeWidget(panel->goBtn, 80, 25);
}

static void CreateHelpFrames(LoginPanel *panel)
{
    int nblines;

    panel->helpF = WMCreateFrame(panel->win);
    WMSetFrameRelief(panel->helpF,WRRaised);
    WMMoveWidget(panel->helpF, 0, P_HEIGTH);
    WMResizeWidget(panel->helpF, P_WIDTH, help_heigth);

    panel->helpSV = WMCreateScrollView(panel->helpF);
    WMResizeWidget(panel->helpSV, (P_WIDTH-10),
				  (help_heigth-10));
    WMMoveWidget(panel->helpSV, 5, 5);
    WMSetScrollViewRelief(panel->helpSV, WRSunken);
    WMSetScrollViewHasVerticalScroller(panel->helpSV, True);
    WMSetScrollViewHasHorizontalScroller(panel->helpSV, False);
#if (WINGS_H_VERSION >= 980901)
    WMSetScrollViewLineScroll(panel->helpSV,12);
#endif

    parse_helpArg();
    nblines = countlines(HelpFile);

    panel->helpTextF = WMCreateFrame(panel->helpF);
    WMSetFrameRelief(panel->helpTextF,WRFlat);

    /* 14 * nblines is far from perfect !!! */
    WMResizeWidget(panel->helpTextF, (P_WIDTH-50), 14 * nblines); /* 620 */

    panel->helpTextL = WMCreateLabel(panel->helpTextF);
    WMSetLabelTextAlignment(panel->helpTextL,WALeft);
    WMMoveWidget(panel->helpTextL, 2, 1);

    /* 14 * nblines is far from perfect !!! */
    WMResizeWidget(panel->helpTextL,(P_WIDTH-60), 14 * nblines - 5); /* 615 */

    WMSetLabelText(panel->helpTextL,HelpFile);
}

static LoginPanel *CreateLoginPanel(WMScreen *scr)
{
    LoginPanel *panel;

    panel = malloc(sizeof(LoginPanel));
    if (!panel)
	return NULL;
    memset(panel, 0, sizeof(LoginPanel));
    panel->scr = scr;

    /* basic window and frames */

    panel->win = WMCreateWindow(scr,ProgName);
    WMResizeWidget(panel->win, panel_width, panel_heigth);

    panel->winF1 = WMCreateFrame(panel->win);
    WMResizeWidget(panel->winF1, panel_width, panel_heigth);
    WMSetFrameRelief(panel->winF1,WRRaised);

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
    WMSetScrollViewContentView(panel->helpSV, WMWidgetView(panel->helpTextF));

    WMSetPopUpButtonSelectedItem(panel->wmBtn,0);
    WMSetPopUpButtonSelectedItem(panel->exitBtn,0);

    panel->msgFlag = False;

    return panel;
}

static void DestroyLoginPanel(LoginPanel *panel)
{
    int width=panel_width, heigth=panel_heigth;
    /* roll up the window before destroying it */
    if (animate) {
	XSynchronize(dpy,True);	 /* slow things up */
	for (width=panel_width-2,heigth=panel_heigth-1;
		(heigth>0 && width>0);
		heigth-=1, width-=2) {
	    WMResizeWidget(panel->win,width,heigth);
	}
	XSynchronize(dpy,False);
    }
    WMUnmapWidget(panel->win);
    WMDestroyWidget(panel->win);
    free(panel);
}

/*###################################################################*/

/** set the background **/

static int parseBG()
{
    char *tmp;

    if (bgArg==NULL)
	return 0;
    tmp = strchr(bgArg,':');
    if (tmp==NULL)
	return 0;
    *tmp = '\0';
    bgOption = tmp+1;
    while (*bgOption==' ')
	bgOption++;
    if (*bgOption=='\0')
	return 0;
    tmp = bgArg;
    while (*tmp!='\0') {
	*tmp = tolower(*tmp);
	tmp++;
    }
    if (strcmp(bgArg,"pixmap")==0)
	return 1;
    if (strcmp(bgArg,"solid")==0)
	return 2;
    if (strcmp(bgArg,"hgradient")==0)
	return 3;
    if (strcmp(bgArg,"vgradient")==0)
	return 4;
    if (strcmp(bgArg,"dgradient")==0)
	return 5;
    return 0;
}

static RImage *loadBGpixmap(RContext *rcontext)
{
    RImage *image, *tmp;

	image = RLoadImage(rcontext, bgOption, 0);
	if (image==NULL) {
	   fprintf(stderr,"%s could not load bg image %s\n",
			ProgName, bgOption);
	   return NULL;
	}
	tmp = RScaleImage(image, screen_width, screen_heigth);
	if (tmp==NULL) {
	   fprintf(stderr,"%s could not resize bg image %s\n",
			ProgName,bgOption);
	   RReleaseImage(image);
	   return NULL;
	}
	RReleaseImage(image);
	return tmp;
}

static RColor **allocmem(int num)
{
    RColor **colors = NULL;
    int i;

    colors = malloc(sizeof(RColor*) * (num+1));
    for (i=0;i<num;i++) {
	colors[i] = malloc(sizeof(RColor));
    }
    colors[i] = NULL;
    return colors;
}

static void freemem(int num, RColor **colors)
{
    int i;
    for (i=0;i<num;i++) {
	free(colors[i]);
    }
    free(colors);
}

static RImage *createBGcolor(RContext *rcontext, char *str, int style)
{
    Window  root_window;
    RImage *image;
    RColor **colors = NULL;
    XColor  color;
    int	    num_colors = 0;
    int	    i;
    char   *tmp, *colorstr;

    root_window = RootWindow(dpy, screen_number);
    colorstr = str;
    while (*colorstr) {
	num_colors++;
	tmp = strchr(colorstr,',');
	if (tmp==NULL)
	    colorstr = str + strlen(str);
	else
	    colorstr = tmp + 1;
    }
    if (num_colors==0)
	return NULL;
    colors = allocmem(num_colors);
    tmp = str;
    for (i=0; i<num_colors; i++) {
	colorstr = tmp;
	tmp = strchr(tmp,',');
	if (tmp!=NULL) {
	    *tmp = '\0';
	    tmp++;
	}
	else
	    tmp = colorstr + strlen(colorstr);
	if (!XParseColor(dpy, rcontext->cmap, colorstr, &color)) {
	    fprintf(stderr,"could not parse color \"%s\"\n",colorstr);
	    freemem(num_colors,colors);
	    return NULL;
	}
	colors[i]->red	 = color.red   >> 8;
	colors[i]->green = color.green >> 8;
	colors[i]->blue	 = color.blue  >> 8;
    }
    image = RRenderMultiGradient(screen_width, screen_heigth,
				 colors, style);
    freemem(num_colors, colors);
    return image;
}

static void setBG()
{
    Window root_window;
    int cpc=4, render_mode = RBestMatchRendering, default_depth=8;
    RContextAttributes rattr;
    RContext *rcontext;
    RImage *image;
    XColor defcolor;
    Pixmap pixmap;

    /* if not specified or none, then skip setting background */
    /* user can still set background via other means */
    if (bgArg==NULL)
	return;
    if (strcasecmp(bgArg,"none")==0)
	return;

    root_window = RootWindow(dpy, screen_number);
    default_depth = DefaultDepth(dpy, screen_number);
    if (default_depth<=8)
	render_mode = RDitheredRendering;
    rattr.flags = RC_RenderMode | RC_ColorsPerChannel;
    rattr.render_mode = render_mode;
    rattr.colors_per_channel = cpc;
    rcontext = RCreateContext(dpy, screen_number, &rattr);
    if (rcontext==NULL) {
	fprintf(stderr,
	   "%s could not initialize graphics library context: %s\n",
	   ProgName, RMessageForError(RErrorCode));
	return;
    }

    defcolor.pixel = 0L;	/* default=black */

    switch (parseBG()) {
	case 1:
	    image = loadBGpixmap(rcontext);
	break;
	case 2:
	    image = createBGcolor(rcontext, bgOption, RGRD_HORIZONTAL);
	break;
	case 3:
	    image = createBGcolor(rcontext, bgOption, RGRD_HORIZONTAL);
	break;
	case 4:
	    image = createBGcolor(rcontext, bgOption, RGRD_VERTICAL);
	break;
	case 5:
	    image = createBGcolor(rcontext, bgOption, RGRD_DIAGONAL);
	break;
	default:
	    image = NULL;
	break;
    }
    if (image==NULL) {
	XSetWindowBackground(dpy, root_window, 0L);
	XClearWindow(dpy, root_window);
	XFlush(dpy);
	return;
    }
    RConvertImage(rcontext, image, &pixmap);
    RReleaseImage(image);
    XSetWindowBackgroundPixmap(dpy, root_window, pixmap);
    XClearWindow(dpy, root_window);
    XFlush(dpy);
}

/*###################################################################*/

/* signal processing */

static void SignalUsr1(int ignored)	/* oops, an error */
{
    char msg[64];

    strcpy(msg,OptionStr);
    strcat(msg," failed.");
    InitializeLoginInput(panel);
    PrintErrMsg(panel,msg);
    signal(SIGUSR1, SignalUsr1);
}

static void SignalTerm(int ignored)	/* all done */
{
    exit_request = 1;	 /* corrects some hanging problems, thanks to A. Kabaev */
}

/*###################################################################*/

/*  M A I N  */

int main(int argc, char **argv)
{
    WMScreen   *scr;

    ProgName = argv[0];

    animate = False;
    LoginArgs(argc, argv);		/* process our args */
    SetupWm();				/* and init the startup list */

    dpy = XOpenDisplay(displayArg);

    if (!dpy) {
	fprintf(stderr,"could not open display\n");
	exit(1);
    }

    screen_number = DefaultScreen(dpy);
    screen_width = DisplayWidth(dpy,screen_number);
    screen_heigth = DisplayHeight(dpy,screen_number);
    panel_X = (screen_width  - panel_width)/2;
    panel_Y = (screen_heigth - panel_heigth)/2;

    XSynchronize(dpy,False);

    XWarpPointer(dpy, None,
		XRootWindowOfScreen(XDefaultScreenOfDisplay(dpy)),
		0, 0, 0, 0,
		(panel_X + (panel_width - 10)),
		(panel_Y + (panel_heigth - 10)));
    XDefineCursor(dpy,
		XRootWindowOfScreen(XDefaultScreenOfDisplay(dpy)),
		XCreateFontCursor(dpy,XC_top_left_arrow));

    setBG();

#if (WINGS_H_VERSION > 980722)
    WMInitializeApplication(ProgName, &argc, argv);
    scr = WMCreateScreen(dpy, screen_number);
#else
    scr = WMCreateScreen(dpy, screen_number, ProgName, NULL, NULL);
#endif
    if (!scr) {
	fprintf(stderr,"could not initialize Screen\n");
	exit(2);
    }

    panel = CreateLoginPanel(scr);
    WMSetWindowTitle(panel->win,ProgName);
    /* the following Resize and the one following the Move fake out WINGs */
    /* so that the move is not visible */
    WMResizeWidget(panel->win,1,1);
    WMMapWidget(panel->win);
    WMSetWindowTitle(panel->win,ProgName);
    WMMoveWidget(panel->win, panel_X,panel_Y);
    WMResizeWidget(panel->win,panel_width,panel_heigth);
    WMSetFocusToWidget(panel->entryText);
    XSetInputFocus(dpy, WMWidgetXID(panel->win), RevertToParent, CurrentTime);
    panel->retkey = XKeysymToKeycode(dpy, XK_Return);
    panel->tabkey = XKeysymToKeycode(dpy, XK_Tab);

    WMCreateEventHandler(WMWidgetView(panel->entryText), KeyPressMask,
				handleKeyPress, panel);

    exit_request = 0;
    signal(SIGUSR1, SignalUsr1);
    signal(SIGTERM, SignalTerm);
    signal(SIGINT,  SignalTerm);
    signal(SIGPIPE, SIG_DFL);

    while (! exit_request) {
	XEvent event;
	WMNextEvent(dpy,&event);
	WMHandleEvent(&event);
    }
    DestroyLoginPanel(panel);

    return 0; /* never get here but keeps compiler happy */
}
