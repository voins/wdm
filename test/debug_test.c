#include <runtest.h>
#include <wdmlib.h>


int main(void)
{
	WDMDebug("\ntest debug functions %i\n\n", 1);
	WDMLogLevel(1);
	WDMDebug("\ntest debug functions %i\n\n", 2);
	WDMLogLevel(0);
	WDMDebug("\ntest debug functions %i\n\n", 3);
	return 0;
}

/*
CFLAGS=-I/usr/X11R6/include
LDFLAGS=-L/usr/X11R6/lib
LIBS=-lWINGs -lwdm
 */
