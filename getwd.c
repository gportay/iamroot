/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
char *next_getwd(char *buf)
{
	char *(*sym)(char *);
	char *ret;

	sym = dlsym(RTLD_NEXT, "getwd");
	if (!sym)
		return __dl_set_errno(ENOSYS, NULL);

	ret = sym(buf);
	if (!ret)
		__pathperror(NULL, __func__);

	return ret;
}

char *getwd(char *buf)
{
	char *ret;

	ret = next_getwd(buf);
	if (!ret) {
		perror(__func__);
		return NULL;
	}

	ret = __striprootdir(ret);
	if (!ret) {
		__pathperror(buf, __func__);
		return NULL;
	}

	__debug("%s(buf: %p)\n", __func__, buf);

	return ret;
}
