#include <runtest.h>
#include <wdmlib.h>

static WDMArraySpec array_of_bool =
	{ WDMCheckPLBool, (void*)False, NULL };

static WDMArraySpec array_of_strings =
	{ WDMCheckPLString, "default", wfree };


void free_test_struct(void *data);

typedef struct _test_struct
{
	Bool boolval;
	char *stringval;
	WMArray *arrayval;
} test_struct;

static WDMDictionaryStruct test_struct_fields[] =
{
	{"bool", WDMCheckPLBool, False,
		offsetof(test_struct, boolval)},
	{"string", WDMCheckPLString, "aaa",
		offsetof(test_struct, stringval)},
	{"array", WDMCheckPLArray, &array_of_strings,
		offsetof(test_struct, arrayval)},
	{NULL, NULL, NULL, 0}
};

static WDMDictionarySpec test_struct_spec =
	{sizeof(test_struct), test_struct_fields};

static WDMArraySpec array_of_structs =
	{WDMCheckPLDictionary, &test_struct_spec, free_test_struct};


int check_array_of_bool(void)
{
	WMPropList *pl = NULL;
	WMArray *array = NULL;

	pl = WMCreatePropListFromDescription("(yes, Yes, no)");
	test_assert(WDMCheckPLArray(pl, &array_of_bool, &array) == True);
	test_assert((Bool)WMGetFromArray(array, 0) == True);
	test_assert((Bool)WMGetFromArray(array, 1) == True);
	test_assert((Bool)WMGetFromArray(array, 2) == False);

	WMFreeArray(array);
	WMReleasePropList(pl);

	return 1;
}

int check_array_of_strings(void)
{
	WMPropList *pl = NULL;
	WMArray *array = NULL;

	pl = WMCreatePropListFromDescription("(yes, Yes, (), no)");
	test_assert(WDMCheckPLArray(pl, &array_of_strings, &array) == True);
	test_assert(strcmp(WMGetFromArray(array, 0), "yes") == 0);
	test_assert(strcmp(WMGetFromArray(array, 1), "Yes") == 0);
	test_assert(strcmp(WMGetFromArray(array, 2), "default") == 0);
	test_assert(strcmp(WMGetFromArray(array, 3), "no") == 0);

	WMFreeArray(array);
	WMReleasePropList(pl);

	return 1;
}

void free_test_struct(void *data)
{
	test_struct *s = data;
	if(s->stringval) 
		wfree(s->stringval);
	if(s->arrayval)
		WMFreeArray(s->arrayval);
	wfree(data);
}

int check_array_of_structs(void)
{
	WMPropList *pl = NULL;
	WMArray *array = NULL;
	test_struct *tst = NULL;

	pl = WMCreatePropListFromDescription(
			"({bool=yes; string=qqq; array=(bbb, ccc);},"
			"{bool=False; array=yyy;})");
	test_assert(WDMCheckPLArray(pl, &array_of_structs, &array) == True);

	test_assert((tst = WMGetFromArray(array, 0)) != NULL);
	test_assert(tst->boolval == True);
	test_assert(strcmp(tst->stringval, "qqq") == 0);
	test_assert(tst->arrayval != NULL);
	test_assert(strcmp(WMGetFromArray(tst->arrayval, 0), "bbb") == 0);
	test_assert(strcmp(WMGetFromArray(tst->arrayval, 1), "ccc") == 0);

	test_assert((tst = WMGetFromArray(array, 1)) != NULL);
	test_assert(tst->boolval == False);
	test_assert(strcmp(tst->stringval, "aaa") == 0);
	test_assert(tst->arrayval == NULL);

	WMFreeArray(array);
	WMReleasePropList(pl);

	return 1;
}

int main(void)
{
	if(check_array_of_bool() &&
		check_array_of_strings() &&
		check_array_of_structs())
			return 0;
	return 1;
}

/*
CFLAGS=-I/usr/X11R6/include
LDFLAGS=-L/usr/X11R6/lib
LIBS=-lwdm -lWINGs
 */
