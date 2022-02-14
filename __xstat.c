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

extern int __rootxstat(int, const char *, struct stat *);

__attribute__((visibility("hidden")))
int next___xstat(int ver, const char *path, struct stat *statbuf)
{
	int (*sym)(int, const char *, struct stat *);
	int ret;

	sym = dlsym(RTLD_NEXT, "__xstat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(ver, path, statbuf);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int __xstat(int ver, const char *path, struct stat *statbuf)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, real_path);

	return __rootxstat(ver, real_path, statbuf);
}
