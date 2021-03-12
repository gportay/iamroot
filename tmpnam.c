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
char *next_tmpnam(char *path)
{
	char *(*sym)(const char *);

	sym = dlsym(RTLD_NEXT, "tmpnam");
	if (!sym) {
		errno = ENOTSUP;
		return NULL;
	}

	return sym(path);
}

char *tmpnam(char *path)
{
	char *real_path;
	char buf[PATH_MAX];

	real_path = (char *)path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next_tmpnam(real_path);
}
