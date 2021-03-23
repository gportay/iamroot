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
int next_removexattr(const char *path, const char *name)
{
	int (*sym)(const char *, const char *);

	sym = dlsym(RTLD_NEXT, "removexattr");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, name);
}

int removexattr(const char *path, const char *name)
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

	return next_removexattr(real_path, name);
}
