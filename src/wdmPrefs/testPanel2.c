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
 * testPanel.c: panel for testing internal wdmPrefs functions.
 */
#include <config.h>
#include <wdmlib.h>
#include <wdmPrefs.h>
#include <stdlib.h>

typedef struct testPanel_
{
	WMBox *box;
	WMButton *button1;
	WMButton *button2;
} testPanel;

static void *
CreateTestPanel(WMWidget *win)
{
	testPanel *data = malloc(sizeof(testPanel));
	data->box = WMCreateBox(win);
	WMSetBoxBorderWidth(data->box, 5);
	WMSetBoxHorizontal(data->box, True);
	WMSetViewExpandsToParent(WMWidgetView(data->box), 0, 0, 0, 0);

	data->button1 = WMCreateSwitchButton(data->box);
	WMMapWidget(data->button1);
	WMSetButtonText(data->button1, "test button 1");
	WMAddBoxSubview(data->box, WMWidgetView(data->button1),
			False, False, 100, 100, 10);

	data->button2 = WMCreateSwitchButton(data->box);
	WMMapWidget(data->button2);
	WMSetButtonText(data->button2, "test button 2");
	WMAddBoxSubview(data->box, WMWidgetView(data->button2),
			False, False, 100, 100, 10);

	return data;
}

static void
DestroyTestPanel(Panel *panel)
{
	free(panel);
}

static void
ShowTestPanel(Panel *panel)
{
	testPanel *data = panel->data;
	WMMapWidget(data->box);
}

static void
HideTestPanel(Panel *panel)
{
	testPanel *data = panel->data;
	WMUnmapWidget(data->box);
}

static void
SaveTestData(Panel *panel)
{
}

static void
UndoTestData(Panel *panel)
{
}

void
InitTestPanel2(WMWidget *win)
{
	Panel *panel = calloc(1, sizeof(Panel));
	panel->description = "simple test panel 2";
	panel->destroy = DestroyTestPanel;
	panel->show = ShowTestPanel;
	panel->hide = HideTestPanel;
	panel->save = SaveTestData;
	panel->undo = UndoTestData;
	panel->data = CreateTestPanel(win);
	AddSectionButton(panel, "");
}

