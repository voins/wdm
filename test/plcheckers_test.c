#include <runtest.h>
#include <wdmlib.h>

int check_bool(void)
{
	WMPropList *pl = NULL;

	test_assert(WDMCheckPLBool(pl, True) == True
			&& WDMCheckPLBool(pl, False) == False);

	pl = WMCreatePropListFromDescription("YES");
	test_assert(WDMCheckPLBool(pl, True) == True
			&& WDMCheckPLBool(pl, False) == True);
	WMReleasePropList(pl);
	
	pl = WMCreatePropListFromDescription("NO");
	test_assert(WDMCheckPLBool(pl, True) == False 
			&& WDMCheckPLBool(pl, False) == False);
	WMReleasePropList(pl);

	pl = WMCreatePropListFromDescription("yes");
	test_assert(WDMCheckPLBool(pl, True) == True 
			&& WDMCheckPLBool(pl, False) == True);
	WMReleasePropList(pl);
	
	pl = WMCreatePropListFromDescription("no");
	test_assert(WDMCheckPLBool(pl, True) == False
			&& WDMCheckPLBool(pl, False) == False);
	WMReleasePropList(pl);

	pl = WMCreatePropListFromDescription("__");
	test_assert(WDMCheckPLBool(pl, True) == True 
			&& WDMCheckPLBool(pl, False) == False);
	WMReleasePropList(pl);

	return 1;
}

int check_string(void)
{
	static char *defval = "default value";
	static char *proper = "proper value";
	static char *properd = "\"proper value\"";
	char *result = NULL;
	WMPropList *pl = NULL;
	
	result = WDMCheckPLString(pl, defval);
	test_assert(strcmp(result, defval) == 0);
	wfree(result);

	pl = WMCreatePropListFromDescription(properd);
	result = WDMCheckPLString(pl, defval);
	test_assert(strcmp(result, proper) == 0);
	wfree(result);
	WMReleasePropList(pl);

	pl = WMCreatePropListFromDescription("(a, b)");
	result = WDMCheckPLString(pl, defval);
	test_assert(strcmp(result, defval) == 0);
	wfree(result);
	WMReleasePropList(pl);
	
	return 1;
}

int check_array(void)
{
	static char *defval = "default";
	WMPropList *pl = NULL;
	WMArray *result = NULL;

	test_assert(WDMCheckPLArray(pl, NULL, NULL) == NULL);
	test_assert(WDMCheckPLArrayWithDestructor(pl, NULL, NULL, NULL)
			== NULL);

	pl = WMCreatePropListFromDescription("\"some string\"");
	test_assert(WDMCheckPLArray(pl, NULL, NULL) == NULL);
	test_assert(WDMCheckPLArrayWithDestructor(pl, NULL, NULL, NULL)
			== NULL);
	WMReleasePropList(pl);

	pl = WMCreatePropListFromDescription("(aaa, bbb, ccc, ())");
	result = WDMCheckPLArrayWithDestructor(pl, wfree,
				WDMUntype(WDMCheckPLString), defval);
	test_assert(WMGetArrayItemCount(result) == 4);
	test_assert(strcmp(WMGetFromArray(result, 0), "aaa") == 0);
	test_assert(strcmp(WMGetFromArray(result, 1), "bbb") == 0);
	test_assert(strcmp(WMGetFromArray(result, 2), "ccc") == 0);
	test_assert(strcmp(WMGetFromArray(result, 3), defval) == 0);
	WMReleasePropList(pl);
	
	return 1;
}


int main(void)
{
	if(check_bool()
		&& check_string()
		&& check_array())
		return 0;
	return 1;
}

/*
CFLAGS=-I/usr/X11R6/include
LDFLAGS=-L/usr/X11R6/lib
LIBS=-lWINGs -lwdm
 */
