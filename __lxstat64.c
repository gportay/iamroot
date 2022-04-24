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

#ifdef __GLIBC__
extern int __rootlxstat64(int, const char *, struct stat64 *);

__attribute__((visibility("hidden")))
int next___lxstat64(int ver, const char *path, struct stat64 *statbuf)
{
	int (*sym)(int, const char *, struct stat64 *);
	int ret;

	sym = dlsym(RTLD_NEXT, "__lxstat64");
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

int __lxstat64(int ver, const char *path, struct stat64 *statbuf)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, buf);

	return __rootlxstat64(ver, buf, statbuf);
}
#endif
