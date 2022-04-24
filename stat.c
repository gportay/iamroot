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

extern int rootstat(const char *, struct stat *);

__attribute__((visibility("hidden")))
int next_stat(const char *path, struct stat *statbuf)
{
	int (*sym)(const char *, struct stat *);
	int ret;

	sym = dlsym(RTLD_NEXT, "stat");
	if (!sym) {
		int next___xstat(int, const char *, struct stat *);
#if defined(__arm__)
		return next___xstat(3, path, statbuf);
#else
		return next___xstat(0, path, statbuf);
#endif
	}

	ret = sym(path, statbuf);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int stat(const char *path, struct stat *statbuf)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, buf);

	return rootstat(buf, statbuf);
}
