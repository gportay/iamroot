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

int next_remove(const char *path)
{
	int (*sym)(const char *);

	sym = dlsym(RTLD_NEXT, "remove");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path);
}

int remove(const char *path)
{
	const char *real_path;
	char buf[PATH_MAX];

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next_remove(real_path);
}
