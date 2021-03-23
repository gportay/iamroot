/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/xattr.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next_setxattr(const char *path, const char *name, const void *value,
		  size_t size, int flags)
{
	int (*sym)(const char *, const char *, const void *, size_t, int);

	sym = dlsym(RTLD_NEXT, "setxattr");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, name, value, size, flags);
}

int setxattr(const char *path, const char *name, const void *value,
	     size_t size, int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s', ...)\n", __func__, path,
			  real_path);

	return next_setxattr(real_path, name, value, size, flags);
}
