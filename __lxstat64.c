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

#ifdef __GLIBC__
extern int __rootlxstat64(int, const char *, struct stat64 *);

__attribute__((visibility("hidden")))
int next___lxstat64(int ver, const char *path, struct stat64 *statbuf)
{
	int (*sym)(int, const char *, struct stat64 *);
	int ret;

	sym = dlsym(RTLD_NEXT, "__lxstat64");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(ver, path, statbuf);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

int __lxstat64(int ver, const char *path, struct stat64 *statbuf)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf),
				    AT_SYMLINK_NOFOLLOW);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__verbose("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	return __rootlxstat64(ver, real_path, statbuf);
}
#endif
