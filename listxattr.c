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

__attribute__((visibility("hidden")))
ssize_t next_listxattr(const char *path, char *list, size_t size)
{
	ssize_t (*sym)(const char *, char *, size_t);

	sym = dlsym(RTLD_NEXT, "listxattr");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, list, size);
}

ssize_t listxattr(const char *path, char *list, size_t size)
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

	return next_listxattr(real_path, list, size);
}
