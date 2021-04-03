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

#ifdef __GLIBC__
extern int __rootxstat64(int, const char *, struct stat64 *);

__attribute__((visibility("hidden")))
int next___xstat64(int ver, const char *path, struct stat64 *stat64buf)
{
	int (*sym)(int, const char *, struct stat64 *);
	int ret;

	sym = dlsym(RTLD_NEXT, "__xstat64");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(ver, path, stat64buf);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

int __xstat64(int ver, const char *path, struct stat64 *stat64buf)
{
	char buf[PATH_MAX];
	char *real_path;
	int ret;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	ret = __rootxstat64(ver, real_path, stat64buf);

	__verbose("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	return ret;
}
#endif
