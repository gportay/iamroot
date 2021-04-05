/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "iamroot.h"

extern char *path_resolution(const char *, char *, size_t, int);
#ifdef __GLIBC__
extern int __rootlxstat64(int, const char *, struct stat64 *);

__attribute__((visibility("hidden")))
int next___lxstat64(int ver, const char *path, struct stat64 *stat64buf)
{
	int (*sym)(int, const char *, struct stat64 *);

	sym = dlsym(RTLD_NEXT, "__lxstat64");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(ver, path, stat64buf);
}

int __lxstat64(int ver, const char *path, struct stat64 *stat64buf)
{
	char buf[PATH_MAX];
	char *real_path;
	int ret;

	real_path = path_resolution(path, buf, sizeof(buf),
				    AT_SYMLINK_NOFOLLOW);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	ret = __rootlxstat64(ver, real_path, stat64buf);

	__verbose("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	return ret;
}
#endif
