/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <stdio.h>

#include "iamroot.h"

extern char *path_resolution(const char *, char *, size_t, int);

__attribute__((visibility("hidden")))
char *next_tmpnam_r(char *path)
{
	char *(*sym)(char *);

	sym = dlsym(RTLD_NEXT, "tmpnam_r");
	if (!sym) {
		errno = ENOSYS;
		return NULL;
	}

	return sym(path);
}

char *tmpnam_r(char *path)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	__verbose("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next_tmpnam_r(real_path);
}
