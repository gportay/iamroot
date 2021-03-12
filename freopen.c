/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <stdio.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

FILE *next_freopen(const char *path, const char *mode, FILE *stream)
{
	FILE *(*sym)(const char *, const char *, FILE *);

	sym = dlsym(RTLD_NEXT, "freopen");
	if (!sym) {
		errno = ENOTSUP;
		return NULL;
	}

	return sym(path, mode, stream);
}

FILE *freopen(const char *path, const char *mode, FILE *stream)
{
	const char *real_path;
	char buf[PATH_MAX];

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next_freopen(real_path, mode, stream);
}
