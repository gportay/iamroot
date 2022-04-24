/*
 * Copyright 2021-2022 Gaël PORTAY
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

#ifdef __GLIBC__
extern int rootlstat64(const char *, struct stat64 *);

__attribute__((visibility("hidden")))
int next_lstat64(const char *path, struct stat64 *statbuf)
{
	int (*sym)(const char *, struct stat64 *);
	int ret;

	sym = dlsym(RTLD_NEXT, "lstat64");
	if (!sym) {
		int next___lxstat64(int, const char *, struct stat64 *);
#if defined(__arm__)
		return next___lxstat64(3, path, statbuf);
#else
		return next___lxstat64(0, path, statbuf);
#endif
	}

	ret = sym(path, statbuf);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int lstat64(const char *path, struct stat64 *statbuf)
{
	char buf[PATH_MAX];

	if (path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			    AT_SYMLINK_NOFOLLOW) == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, buf);

	return rootlstat64(buf, statbuf);
}
#endif
