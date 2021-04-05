/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>

#include <dlfcn.h>

#include "iamroot.h"

extern char *path_resolution(const char *, char *, size_t, int);

__attribute__((visibility("hidden")))
void *next_dlopen(const char *path, int flags)

{
	void *(*sym)(const char *, int);

	sym = dlsym(RTLD_NEXT, "dlopen");
	if (!sym) {
		errno = ENOSYS;
		return NULL;
	}

	return sym(path, flags);
}

void *dlopen(const char *path, int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	__verbose("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next_dlopen(path, flags);
}
