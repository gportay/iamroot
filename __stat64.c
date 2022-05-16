/*
 * Copyright 2022 Gaël PORTAY
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
extern int __rootstat64(const char *, struct stat64 *);

__attribute__((visibility("hidden")))
int next___stat64(const char *path, struct stat64 *statbuf)
{
	int (*sym)(const char *, struct stat64 *);
	int ret;

	sym = dlsym(RTLD_NEXT, "__stat64");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(path, statbuf);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int __stat64(const char *path, struct stat64 *statbuf)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, buf);

	return __rootstat64(buf, statbuf);
}

weak_alias(__stat64, __stat_time64);
#endif
