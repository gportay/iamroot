/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <stdio.h>

#include "iamroot.h"

extern char *path_resolution(const char *, char *, size_t, int);
#ifdef __GLIBC__

__attribute__((visibility("hidden")))
FILE *next_fopen64(const char *path, const char *mode)
{
	FILE *(*sym)(const char *, const char *);

	sym = dlsym(RTLD_NEXT, "fopen64");
	if (!sym) {
		errno = ENOSYS;
		return NULL;
	}

	return sym(path, mode);
}

FILE *fopen64(const char *path, const char *mode)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	__verbose("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next_fopen64(real_path, mode);
}
#endif
