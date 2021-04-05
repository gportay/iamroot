/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <utime.h>

#include "iamroot.h"

extern char *path_resolution(const char *, char *, size_t, int);

__attribute__((visibility("hidden")))
int next_utime(const char *path, const struct utimbuf *times)
{
	int (*sym)(const char *, const struct utimbuf *);

	sym = dlsym(RTLD_NEXT, "utime");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(path, times);
}

int utime(const char *path, const struct utimbuf *times)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__verbose("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next_utime(real_path, times);
}
