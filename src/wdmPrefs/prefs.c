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
 * prefs.c: wdmPrefs program. configuration tool for wdm
 */
#include <config.h>
#include <wdmlib.h>
#include <stdlib.h>

typedef struct _wdmPrefsPanel
{
	WMWindow *win;
	WMFrame *fsects;
	WMArray *sections;
} wdmPrefsPanel;
wdmPrefsPanel wdmPrefs = {NULL, NULL, NULL};

typedef struct _Panel
{
	WMBox *box;
	char *description;
} Panel;

static void
ChangeSection(WMWidget *self, void *data)
{
}

static void
DestroyButton(void *data)
{
}

void
AddSectionButton(Panel *panel, const char *iconfile)
{
	WMButton *button;
	char *filename;
	RColor color;
	WMPixmap *icon;

	button = WMCreateCustomButton(wdmPrefs.fsects,
			WBBStateLightMask | WBBStateChangeMask);
	WMResizeWidget(button, 70, 70);
	WMMoveWidget(button, WMGetArrayItemCount(wdmPrefs.sections) *
			WMWidgetWidth(button), 0);
	WMSetButtonImagePosition(button, WIPImageOnly);
	WMSetButtonAction(button, ChangeSection, panel);
	WMHangData(button, panel);

	if(panel && panel->description)
		WMSetBalloonTextForView(panel->description,
				WMWidgetView(button));

	WMAddToArray(wdmPrefs.sections, button);
	if(WMGetArrayItemCount(wdmPrefs.sections) > 1)
		WMGroupButtons(WMGetFromArray(wdmPrefs.sections, 0), button);

	WMResizeWidget(wdmPrefs.fsects,
			WMGetArrayItemCount(wdmPrefs.sections) * 70, 70);

	if(iconfile)
	{
		color.red = 0xae;
		color.green = 0xaa;
		color.blue = 0xae;
		color.alpha = 0;
		filename = WMPathForResourceOfType((char*)iconfile, NULL);
		if(filename)
		{
			icon = WMCreateBlendedPixmapFromFile(
				WMWidgetScreen(button), filename, &color);
			WMSetButtonImage(button, icon);
			if(icon) WMReleasePixmap(icon);
			wfree(filename);
		}
	}

	WMMapWidget(button);
}

void
CloseAction(WMWidget *self, void *data)
{
	WMDestroyWidget(self);
	exit(0);
}

static WMFrame *
CreateSectionsSelector(WMBox *box)
{
	WMFrame *frame;
	WMScrollView *scrollv;
	WMScroller *scroller;

	frame = WMCreateFrame(box);
	WMMapWidget(frame);
	WMSetFrameRelief(frame, WRRaised);

	scrollv = WMCreateScrollView(frame);
	WMMapWidget(scrollv);
	WMSetScrollViewRelief(scrollv, WRSunken);
	WMSetViewExpandsToParent(WMWidgetView(scrollv), 10, 10, 10, 10);

	wdmPrefs.fsects = WMCreateFrame(scrollv);
	WMResizeWidget(wdmPrefs.fsects, 500, 70);
	WMMapWidget(wdmPrefs.fsects);
	WMSetFrameRelief(wdmPrefs.fsects, WRFlat);
	WMSetScrollViewContentView(scrollv, WMWidgetView(wdmPrefs.fsects));

	WMSetScrollViewHasHorizontalScroller(scrollv, True);
	WMSetScrollViewHasVerticalScroller(scrollv, False);
	scroller = WMGetScrollViewHorizontalScroller(scrollv);
	WMSetScrollerArrowsPosition(scroller, WSANone);

	return frame;
}

static WMFrame *
CreateButtons(WMBox *box)
{
	WMFrame *fcontrols;
	WMBox *buttonbox;
	WMButton *bsave, *bclose;

	fcontrols = WMCreateFrame(box);
	WMMapWidget(fcontrols);
	WMSetFrameRelief(fcontrols, WRRaised);

	buttonbox = WMCreateBox(fcontrols);
	WMMapWidget(buttonbox);
	WMSetBoxBorderWidth(buttonbox, 0);
	WMSetViewExpandsToParent(WMWidgetView(buttonbox), 10, 10, 10, 10);
	WMSetBoxHorizontal(buttonbox, True);

	bsave = WMCreateCommandButton(buttonbox);
	WMMapWidget(bsave);
	WMSetButtonText(bsave, "Save");
	WMAddBoxSubviewAtEnd(buttonbox, WMWidgetView(bsave), False, False, 100, 100, 0);

	bclose = WMCreateCommandButton(buttonbox);
	WMMapWidget(bclose);
	WMSetButtonText(bclose, "Close");
	WMSetButtonAction(bclose, CloseAction, NULL);
	WMAddBoxSubviewAtEnd(buttonbox, WMWidgetView(bclose), False, False, 100, 100, 10);

	return fcontrols;
}

static WMFrame *
CreateSections(WMBox *box)
{
	WMFrame *frame;

	frame = WMCreateFrame(box);
	WMMapWidget(frame);
	WMSetFrameRelief(frame, WRFlat);

	AddSectionButton(NULL, NULL);
	AddSectionButton(NULL, NULL);

	/* TODO: here will be calls to initers
	 * of various sections with parent = frame. */

	return frame;
}

static void
CreatePrefsWindow(WMScreen *scr)
{
	WMBox *box;

	wdmPrefs.win = WMCreateWindow(scr, "wdmPrefs");
	WMSetWindowCloseAction(wdmPrefs.win, CloseAction, NULL);
	WMSetWindowTitle(wdmPrefs.win, "wdmPrefs");

	box = WMCreateBox(wdmPrefs.win);
	WMMapWidget(box);
	WMSetBoxBorderWidth(box, 0);
	WMSetViewExpandsToParent(WMWidgetView(box), 0, 0, 0, 0);

	WMAddBoxSubview(box, WMWidgetView(CreateSectionsSelector(box)),
			False, False, 113, 113, 0);

	WMAddBoxSubview(box, WMWidgetView(CreateSections(box)),
			True, True, 0, 0, 0);

	WMAddBoxSubview(box, WMWidgetView(CreateButtons(box)),
			False, False, 50, 50, 0);
}

int main(int argc, char *argv[])
{
	WMScreen *scr;

	wdmPrefs.sections = WMCreateArrayWithDestructor(0, DestroyButton);
	WMInitializeApplication("wdmPrefs", &argc, argv);
	scr = WMOpenScreen(NULL);
	if(scr == NULL)
		WDMPanic("could not initialize Screen");

	WMSetResourcePath(WGFXDIR);

	CreatePrefsWindow(scr);
	WMResizeWidget(wdmPrefs.win, 600, 400);
	WMMoveWidget(wdmPrefs.win, 100, 100);

	WMRealizeWidget(wdmPrefs.win);
	WMMapSubwidgets(wdmPrefs.win);
	WMMapWidget(wdmPrefs.win);

	WMScreenMainLoop(scr);

	return 0;
}

