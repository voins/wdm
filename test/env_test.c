#include <runtest.h>
#include <wdmlib.h>

int check_working_environment(void)
{
	char **env = NULL;
	env = WDMPutEnv(env, "aaa=bbb");
	test_assert(env != NULL);
	test_assert(strcmp(WDMGetEnv((const char **)env, "aaa"), "bbb") == 0);
	test_assert(strcmp(env[0], "aaa=bbb") == 0);
	test_assert(env[1] == NULL);

	env = WDMSetEnv(env, "aaa", "ccc");
	test_assert(strcmp(WDMGetEnv((const char **)env, "aaa"), "ccc") == 0);
	test_assert(strcmp(env[0], "aaa=ccc") == 0);
	test_assert(env[1] == NULL);
	
	env = WDMSetEnv(env, "ddd", "eee");
	test_assert(strcmp(WDMGetEnv((const char **)env, "aaa"), "ccc") == 0);
	test_assert(strcmp(WDMGetEnv((const char **)env, "ddd"), "eee") == 0);
	test_assert(strcmp(env[0], "aaa=ccc") == 0);
	test_assert(strcmp(env[1], "ddd=eee") == 0);
	test_assert(env[2] == NULL);

	env = WDMUnsetEnv(env, "aaa");
	test_assert(strcmp(env[0], "ddd=eee") == 0);
	test_assert(env[1] == NULL);

	env = WDMUnsetEnv(env, "ddd");
	test_assert(env[0] == NULL);

	WDMFreeEnv(env);

	return 1;
}

int main(void)
{
/*	WDMLogLevel(WDM_LEVEL_DEBUG);*/
	if(check_working_environment())
		return 0;
	return 1;
}

/*
CFLAGS=-I/usr/X11R6/include
LDFLAGS=-L/usr/X11R6/lib
LIBS=-lwdm -lWINGs
 */
