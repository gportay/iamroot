/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

int next_rmdir(const char *path)
{
	int (*sym)(const char *);

	sym = dlsym(RTLD_NEXT, "rmdir");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path);
}

int rmdir(const char *path)
{
	const char *real_path = path;
	char buf[PATH_MAX];

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next_rmdir(real_path);
}
