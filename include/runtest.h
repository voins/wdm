/**
 * Copyright (C) 2002, 2003 Alexey Voinov <voins@voins.program.ru>
 *
 * @file
 * Вспомогательные макросы для создания тестов
 */
#ifndef __RUNTEST_H
#define __RUNTEST_H
#include <stdio.h>

#define test_assert(x) \
	if(!(x)) \
	{ \
		printf("assertion failed: %s in line %i\n", #x, __LINE__); \
		return 0; \
	}

#endif /* __RUNTEST_H */

