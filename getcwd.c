/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
char *next_getcwd(char *buf, size_t size)
{
	char *(*sym)(char *, size_t);
	char *ret;

	sym = dlsym(RTLD_NEXT, "getcwd");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(buf, size);
	if (!ret)
		__pathperror(NULL, __func__);

	return ret;
}

char *getcwd(char *buf, size_t size)
{
	char *ret;

	ret = next_getcwd(buf, size);
	if (!ret) {
		perror(__func__);
		return NULL;
	}

	ret = __striprootdir(ret);
	if (!ret) {
		__pathperror(buf, __func__);
		return NULL;
	}

	__debug("%s(buf: %p, size: %lu)\n", __func__, buf, (unsigned long)size);

	return ret;
}
