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
extern int rootfstatat64(int, const char *, struct stat64 *, int);

__attribute__((visibility("hidden")))
int next_fstatat64(int fd, const char *path, struct stat64 *statbuf, int flags)
{
	int (*sym)(int, const char *, struct stat64 *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "fstatat64");
	if (!sym) {
		int next___fxstatat64(int, int, const char *, struct stat64 *,
				      int);
#if defined(__arm__)
		return next___fxstatat64(3, fd, path, statbuf, flags);
#else
		return next___fxstatat64(0, fd, path, statbuf, flags);
#endif
	}

	ret = sym(fd, path, statbuf, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int fstatat64(int fd, const char *path, struct stat64 *statbuf, int flags)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(fd, path, buf, sizeof(buf), flags);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(fd: %i, path: '%s' -> '%s', ..., flags: 0x%x)\n", __func__,
		fd, path, buf, flags);

	return rootfstatat64(fd, buf, statbuf, flags);
}

weak_alias(fstatat64, __fstatat64);
weak_alias(fstatat64, __fstatat_time64);
#endif
