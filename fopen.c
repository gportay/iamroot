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

extern char *path_resolution(const char *, char *, size_t, int);
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
FILE *next_fopen(const char *path, const char *mode)
{
	FILE *(*sym)(const char *, const char *);

	sym = dlsym(RTLD_NEXT, "fopen");
	if (!sym) {
		errno = ENOSYS;
		return NULL;
	}

	return sym(path, mode);
}

FILE *fopen(const char *path, const char *mode)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next_fopen(real_path, mode);
}
