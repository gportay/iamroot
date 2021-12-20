/*
 * Copyright 2021 Gaël PORTAY
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

extern int __rootfxstatat(int, int, const char *, struct stat *, int);

__attribute__((visibility("hidden")))
int next___fxstatat(int ver, int fd, const char *path, struct stat *statbuf,
		    int flags)
{
	int (*sym)(int, int, const char *, struct stat *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "__fxstatat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(ver, fd, path, statbuf, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int __fxstatat(int ver, int fd, const char *path, struct stat *statbuf,
	       int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		__pathperror(path, "fpath_resolutionat");
		return -1;
	}

	__debug("%s(fd: %i, path: '%s' -> '%s', ...)\n", __func__, fd, path,
		real_path);

	return __rootfxstatat(ver, fd, real_path, statbuf, flags);
}
