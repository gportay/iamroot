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

#include <fcntl.h>
#include <sys/stat.h>

#include "path_resolution.h"

#ifdef __GLIBC__
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern int lrootstat64(const char *, struct stat64 *);

__attribute__((visibility("hidden")))
int next_lstat64(const char *path, struct stat64 *stat64buf)
{
	int (*sym)(const char *, struct stat64 *);

	sym = dlsym(RTLD_NEXT, "lstat64");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, stat64buf);
}

int lstat64(const char *path, struct stat64 *stat64buf)
{
	char buf[PATH_MAX];
	char *real_path;
	int ret;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	ret = lrootstat64(real_path, stat64buf);

	__fprintf(stderr, "%s(path: '%s' -> '%s', ...)\n", __func__, path,
			  real_path);

	return ret;
}
#endif
