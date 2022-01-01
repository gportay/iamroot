/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

#ifdef __GLIBC__
extern int rootstat64(const char *, struct stat64 *);

__attribute__((visibility("hidden")))
int next_stat64(const char *path, struct stat64 *statbuf)
{
	int (*sym)(const char *, struct stat64 *);
	int ret;

	sym = dlsym(RTLD_NEXT, "stat64");
	if (!sym) {
		int next___xstat64(int, const char *, struct stat64 *);
#if defined(__arm__)
		return next___xstat64(3, path, statbuf);
#else
		return next___xstat64(0, path, statbuf);
#endif
	}

	ret = sym(path, statbuf);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int stat64(const char *path, struct stat64 *statbuf)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, "path_resolution");
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	return rootstat64(real_path, statbuf);
}
#endif
