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
#include <sys/stat.h>
#include <fcntl.h>

#include "path_resolution.h"

#ifdef __GLIBC__
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next___open64_2(const char *path, int flags)
{
	int (*sym)(const char *, int);

	sym = dlsym(RTLD_NEXT, "__open64_2");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, flags);
}

int __open64_2(const char *path, int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next___open64_2(real_path, flags);
}
#endif
