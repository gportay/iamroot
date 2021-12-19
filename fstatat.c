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

extern int rootfstatat(int, const char *, struct stat *, int);

__attribute__((visibility("hidden")))
int next_fstatat(int fd, const char *path, struct stat *statbuf, int flags)
{
#if defined __GLIBC__ && !__GLIBC_PREREQ(2,33)
	int next___fxstatat(int, int, const char *, struct stat *, int);
	return next___fxstatat(_STAT_VER, fd, path, statbuf, flags);
#else
	int (*sym)(int, const char *, struct stat *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "fstatat");
	if (!sym) {
		int next___fxstatat(int, int, const char *, struct stat *,
				    int);
		__dl_perror(__func__);
		return next___fxstatat(0, fd, path, statbuf, flags);
	}

	ret = sym(fd, path, statbuf, flags);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
#endif
}

int fstatat(int fd, const char *path, struct stat *statbuf, int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__debug("%s(fd: %i, path: '%s' -> '%s', ...)\n", __func__, fd, path,
		real_path);

	return rootfstatat(fd, real_path, statbuf, flags);
}
