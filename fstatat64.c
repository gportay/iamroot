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

#ifndef __GLIBC_PREREQ
#define __GLIBC_PREREQ(maj,min) 0
#endif

#ifdef __GLIBC__
extern int rootfstatat64(int, const char *, struct stat64 *, int);

__attribute__((visibility("hidden")))
int next_fstatat64(int fd, const char *path, struct stat64 *statbuf, int flags)
{
#if defined __GLIBC__ && !__GLIBC_PREREQ(2,33)
	int next___fxstatat64(int, int, const char *, struct stat64 *, int);
	return next___fxstatat64(_STAT_VER, fd, path, statbuf, flags);
#else
	int (*sym)(int, const char *, struct stat64 *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "fstatat64");
	if (!sym) {
		int next___fxstatat64(int, int, const char *, struct stat64 *,
				      int);
		__dlperror(__func__);
		return next___fxstatat64(0, fd, path, statbuf, flags);
	}

	ret = sym(fd, path, statbuf, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
#endif
}

int fstatat64(int fd, const char *path, struct stat64 *statbuf, int flags)
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

	return rootfstatat64(fd, real_path, statbuf, flags);
}
#endif
