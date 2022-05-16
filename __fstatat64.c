/*
 * Copyright 2022 Gaël PORTAY
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
extern int __rootfstatat64(int, const char *, struct stat64 *, int);

__attribute__((visibility("hidden")))
int next___fstatat64(int fd, const char *path, struct stat64 *statbuf, int flags)
{
	int (*sym)(int, const char *, struct stat64 *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "__fstatat64");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, statbuf, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int __fstatat64(int fd, const char *path, struct stat64 *statbuf, int flags)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(fd, path, buf, sizeof(buf), flags);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(fd: %i, path: '%s' -> '%s', ..., flags: 0x%x)\n", __func__,
		fd, path, buf, flags);

	return __rootfstatat64(fd, buf, statbuf, flags);
}

weak_alias(__fstatat64, __fstatat_time64);
#endif
