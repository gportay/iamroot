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

__attribute__((visibility("hidden")))
int next_utime(const char *path, const struct utimbuf *times)
{
	int (*sym)(const char *, const struct utimbuf *);
	int ret;

	sym = dlsym(RTLD_NEXT, "utime");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, times);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
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
