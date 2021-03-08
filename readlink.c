/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...);

ssize_t next_readlink(const char *path, char *buf, size_t bufsize)
{
	ssize_t (*sym)(const char *, char *, size_t);

	sym = dlsym(RTLD_NEXT, "readlink");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, buf, bufsize);
}

ssize_t readlink(const char *path, char *buf, size_t bufsize)
{
	const char *real_path;
	char tmp[PATH_MAX];

	real_path = path_resolution(path, tmp, sizeof(tmp), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s', ...)\n", __func__, path,
			  real_path);

	return next_readlink(real_path, buf, bufsize);
}
