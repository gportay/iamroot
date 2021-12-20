/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

#ifndef __GLIBC_PREREQ
#define __GLIBC_PREREQ(maj,min) 0
#endif

extern int rootlstat(const char *, struct stat *);

__attribute__((visibility("hidden")))
int next_lstat(const char *path, struct stat *statbuf)
{
#if defined __GLIBC__ && !__GLIBC_PREREQ(2,33)
	int next___lxstat(int, const char *, struct stat *);
	return next___lxstat(_STAT_VER, path, statbuf);
#else
	int (*sym)(const char *, struct stat *);
	int ret;

	sym = dlsym(RTLD_NEXT, "lstat");
	if (!sym) {
		int next___lxstat(int, const char *, struct stat *);
		__dlperror(__func__);
		return next___lxstat(0, path, statbuf);
	}

	ret = sym(path, statbuf);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
#endif
}

int lstat(const char *path, struct stat *statbuf)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf),
				    AT_SYMLINK_NOFOLLOW);
	if (!real_path) {
		__pathperror(path, "path_resolution");
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	return rootlstat(real_path, statbuf);
}
