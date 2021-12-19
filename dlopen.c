/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>

#include <dlfcn.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
void *next_dlopen(const char *path, int flags)

{
	void *(*sym)(const char *, int);
	void *ret;

	sym = dlsym(RTLD_NEXT, "dlopen");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return NULL;
	}

	ret = sym(path, flags);
	if (!ret)
		__perror(path, __func__);

	return ret;
}

void *dlopen(const char *path, int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		__perror(path, "path_resolution");
		return NULL;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next_dlopen(path, flags);
}
