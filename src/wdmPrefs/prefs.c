#include <wdmlib.h>

void
closeAction(WMWidget *self, void *data)
{
	WMDestroyWidget(self);
	exit(0);
}

int main(int argc, char *argv[])
{
	WMScreen *scr;
	WMWindow *win;

	WMInitializeApplication("wdmPrefs", &argc, argv);
	scr = WMOpenScreen(NULL);
	if(scr == NULL)
		WDMPanic("could not initialize Screen");


	win = WMCreateWindow(scr, "wdmPrefs");
	WMResizeWidget(win, 600, 300);
	WMMoveWidget(win, 100, 100);
	WMRealizeWidget(win);
	WMMapWidget(win);
	WMSetWindowCloseAction(win, closeAction, NULL);
	WMSetWindowTitle(win, "wdmPrefs");
	WMScreenMainLoop(scr);

	return 0;
}

