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

extern int rootfstatat(int, const char *, struct stat *, int);

__attribute__((visibility("hidden")))
int next_fstatat(int fd, const char *path, struct stat *statbuf, int flags)
{
	int (*sym)(int, const char *, struct stat *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "fstatat");
	if (!sym) {
		int next___fxstatat(int, int, const char *, struct stat *,
				    int);
#if defined(__arm__)
		return next___fxstatat(3, fd, path, statbuf, flags);
#else
		return next___fxstatat(0, fd, path, statbuf, flags);
#endif
	}

	ret = sym(fd, path, statbuf, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int fstatat(int fd, const char *path, struct stat *statbuf, int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(fd: %i, path: '%s' -> '%s', ..., flags: 0x%x)\n", __func__,
		fd, path, real_path, flags);

	return rootfstatat(fd, real_path, statbuf, flags);
}
