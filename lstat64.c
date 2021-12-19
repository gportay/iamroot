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

#include <sys/stat.h>

#include "iamroot.h"

#ifndef __GLIBC_PREREQ
#define __GLIBC_PREREQ(maj,min) 0
#endif

#ifdef __GLIBC__
extern int rootlstat64(const char *, struct stat64 *);

__attribute__((visibility("hidden")))
int next_lstat64(const char *path, struct stat64 *statbuf)
{
#if defined __GLIBC__ && !__GLIBC_PREREQ(2,33)
	int next___lxstat64(int, const char *, struct stat64 *);
	return next___lxstat64(_STAT_VER, path, statbuf);
#else
	int (*sym)(const char *, struct stat64 *);
	int ret;

	sym = dlsym(RTLD_NEXT, "lstat64");
	if (!sym) {
		int next___lxstat64(int, const char *, struct stat64 *);
		__dl_perror(__func__);
		return next___lxstat64(0, path, statbuf);
	}

	ret = sym(path, statbuf);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
#endif
}

int lstat64(const char *path, struct stat64 *statbuf)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf),
				    AT_SYMLINK_NOFOLLOW);
	if (!real_path) {
		__perror(path, "path_resolution");
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	return rootlstat64(real_path, statbuf);
}
#endif
