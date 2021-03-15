/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <stdio.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
char *next_tempnam(const char *path, const char *pfx)
{
	char *(*sym)(const char *, const char *);

	sym = dlsym(RTLD_NEXT, "tempnam");
	if (!sym) {
		errno = ENOTSUP;
		return NULL;
	}

	return sym(path, pfx);
}

char *tempnam(const char *path, const char *pfx)
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

	return next_tempnam(real_path, pfx);
}
