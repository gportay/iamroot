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

extern char *fpath_resolutionat(int, const char *, char *, size_t, int);
#ifdef __GLIBC__
extern int __rootfxstatat64(int, int, const char *, struct stat *, int);

__attribute__((visibility("hidden")))
int next___fxstatat64(int ver, int fd, const char *path, struct stat *statbuf,
		    int flags)
{
	int (*sym)(int, int, const char *, struct stat *, int);

	sym = dlsym(RTLD_NEXT, "__fxstatat64");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	return sym(ver, fd, path, statbuf, flags);
}

int __fxstatat64(int ver, int fd, const char *path, struct stat *statbuf,
	       int flags)
{
	char buf[PATH_MAX];
	char *real_path;
	int ret;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	ret = __rootfxstatat64(ver, fd, real_path, statbuf, flags);

	__verbose("%s(fd: %i, path: '%s' -> '%s', ...)\n", __func__, fd, path,
		  real_path);

	return ret;
}
#endif
