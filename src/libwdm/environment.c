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
 * environment.c: some functions to work with environments
 */
#include <wdmlib.h>
#include <string.h>

const char *
WDMGetEnv(const char **env, const char *name)
{
	size_t len;
	
	WDMDebug("WDMGetEnv env=%p, name=\"%s\"\n", (void *)env, name);

	if(name == NULL)
	{
		WDMError("WDMGetEnv: internal error: name == NULL\n");
		return NULL;
	}

	if(env == NULL || name[0] == '\0')
		return NULL;

	len = strlen(name);

	while(*env)
	{
		if(!strncmp(*env, name, len) && (*env)[len] == '=')
			return &(*env)[len + 1];
		++env;
	}
	
	return NULL;
}

char **
WDMPutEnv(char **env, const char *string)
{
	const char *name_end;
	char **ep;
	int envsize;
	size_t len;

	WDMDebug("WDMPutEnv env=%p, string=\"%s\"\n", (void *)env, string);
	
	if(string == NULL)
	{
		WDMError("WDMPutEnv: internal error: string == NULL\n");
		return env;
	}

	if(string[0] == '\0')
		return env;

	if((name_end = strchr(string, '=')) != NULL)
	{
		if(name_end == string)
			return env;

		ep = env;
		len = name_end - string;
		while(ep && *ep)
		{
			if(!strncmp(*ep, string, len) && (*ep)[len] == '=')
				break;
			++ep;
		}

		if(ep == NULL || *ep == NULL)
		{
			for(envsize = 0; env && env[envsize] != 0; ++envsize)
				/*nothing*/;
			WDMDebug("WDMPutEnv: realloc env to size %i\n",
					envsize + 2);
			env = wrealloc(env, (envsize + 2) * sizeof(char *));
			env[envsize++] = wstrdup((char *)string);
			env[envsize] = NULL;
		}
		else
		{
			wfree(*ep);
			*ep = wstrdup((char *)string);
		}
		return env;
	}
	return env;
}

char **
WDMSetEnv(char **env, const char *name, const char *value)
{
	char *string;

	WDMDebug("WDMSetEnv env=%p, name=\"%s\", value=\"%s\"\n",
			(void *)env, name, value);
	
	if(name == NULL || name[0] == '\0')
	{
		WDMError("WDMSetEnv: internal error: name == NULL or empty\n");
		return env;
	}
	
	if(value == NULL)
	{
		WDMError("WDMSetEnv: internal error: value == NULL\n");
		return env;
	}

	string = wstrdup((char *)name);
	string = wstrappend(string, "=");
	string = wstrappend(string, (char *)value);
	
	env = WDMPutEnv(env, string);
	
	wfree(string);

	return env;
}

char **
WDMUnsetEnv(char **env, const char *name)
{
	char **ep;
	int envsize;
	size_t len;

	WDMDebug("WDMUnsetEnv env=%p, name=\"%s\"\n", (void *)env, name);

	if(name == NULL)
	{
		WDMError("WDMUnsetEnv: internal error: name == NULL\n");
		return 0;
	}

	if(env == NULL || name[0] == '\0')
		return 0;

	ep = env;
	len = strlen(name);
	while(*ep)
	{
		if(!strncmp(*ep, name, len) && (*ep)[len] == '=')
			break;
		++ep;
	}
	
	if(ep == NULL || *ep == NULL)
		return 0;

	for(envsize = 0; ep[envsize] != 0; ++envsize)
		/*nothing*/;
	WDMDebug("WDMUnsetEnv: moving %i items of %p from %p to %p \n",
			envsize, (void*)env, (void*)(ep + 1), (void*)ep);
	memmove(ep, ep + 1, envsize * sizeof(char *));
	
	for(envsize = 0; env[envsize] != 0; ++envsize)
		/*nothing*/;

	WDMDebug("WDMUnsetEnv: realloc env to size %i\n", envsize + 1);
	return wrealloc(env, (envsize + 1) * sizeof(char*));
}

int
WDMFreeEnv(char **env)
{
	char **ep = env;

	WDMDebug("WDMFreeEnv env=%p\n", (void *)env);
	
	if(env == NULL)
		return 0;
	
	while(*ep)
	{
		wfree(*ep);
		ep++;
	}

	wfree(env);
}

void
WDMPrintEnv(char **env)
{
	WDMDebug("WDMPrintEnv env=%p\n", (void *)env);

	while(env && *env)
		WDMDebug("%s\n", *env++);
}

