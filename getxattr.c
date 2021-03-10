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

#include <sys/types.h>
#include <sys/xattr.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

ssize_t next_getxattr(const char *path, const char *name, void *value,
		      size_t size)
{
	ssize_t (*sym)(const char *, const char *, void *, size_t);

	sym = dlsym(RTLD_NEXT, "getxattr");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, name, value, size);
}

ssize_t getxattr(const char *path, const char *name, void *value, size_t size)
{
	const char *real_path;
	char buf[PATH_MAX];

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s', ...)\n", __func__, path,
			  real_path);

	return next_getxattr(real_path, name, value, size);
}
