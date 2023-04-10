/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
char *next_get_current_dir_name()
{
	char *(*sym)();
	char *ret;

	sym = dlsym(RTLD_NEXT, "get_current_dir_name");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym();
	if (!ret)
		__pathperror(NULL, __func__);

	return ret;
}

char *get_current_dir_name()
{
	char *ret;

	ret = next_get_current_dir_name();
	if (!ret) {
		perror(__func__);
		return NULL;
	}

	ret = __striprootdir(ret);
	if (!ret) {
		__pathperror(NULL, __func__);
		return NULL;
	}

	__debug("%s()\n", __func__);

	return ret;
}
