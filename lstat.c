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

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

extern int rootlstat(const char *, struct stat *);

__attribute__((visibility("hidden")))
int next_lstat(const char *path, struct stat *statbuf)
{
	int (*sym)(const char *, struct stat *);
	int ret;

	sym = dlsym(RTLD_NEXT, "lstat");
	if (!sym) {
		int next___lxstat(int, const char *, struct stat *);
#if defined(__arm__)
		return next___lxstat(3, path, statbuf);
#else
		return next___lxstat(0, path, statbuf);
#endif
	}

	ret = sym(path, statbuf);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int lstat(const char *path, struct stat *statbuf)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
				    AT_SYMLINK_NOFOLLOW);
	if (!real_path) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	return rootlstat(real_path, statbuf);
}
