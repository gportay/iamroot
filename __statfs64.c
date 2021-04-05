/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/statfs.h>

#include "iamroot.h"

extern char *path_resolution(const char *, char *, size_t, int);
#ifdef __GLIBC__

__attribute__((visibility("hidden")))
int next___statfs64(const char *path, struct statfs64 *statfs64buf)
{
	int (*sym)(const char *, struct statfs64 *);

	sym = dlsym(RTLD_NEXT, "__statfs64");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(path, statfs64buf);
}

int __statfs64(const char *path, struct statfs64 *statfs64buf)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__verbose("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	return next___statfs64(real_path, statfs64buf);
}
#endif
