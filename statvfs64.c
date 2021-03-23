/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/statvfs.h>

#include "path_resolution.h"

#ifdef __GLIBC__
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next_statvfs64(const char *path, struct statvfs64 *statvfs64buf)
{
	int (*sym)(const char *, struct statvfs64 *);

	sym = dlsym(RTLD_NEXT, "statvfs64");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, statvfs64buf);
}

int statvfs64(const char *path, struct statvfs64 *statvfs64buf)
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

	return next_statvfs64(real_path, statvfs64buf);
}
#endif
