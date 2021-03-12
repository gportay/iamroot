/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <sys/stat.h>

#include "fpath_resolutionat.h"

#ifdef __GLIBC__
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern int frootstatat64(int, const char *, struct stat64 *, int);

__attribute__((visibility("hidden")))
int next_fstatat64(int fd, const char *path, struct stat *statbuf, int flags)
{
	int (*sym)(int, const char *, struct stat *, int);

	sym = dlsym(RTLD_NEXT, "fstatat64");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(fd, path, statbuf, flags);
}

int fstatat64(int fd, const char *path, struct stat64 *statbuf, int flags)
{
	char buf[PATH_MAX];
	char *real_path;
	int ret;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	ret = frootstatat64(fd, real_path, statbuf, flags);

	__fprintf(stderr, "%s(fd: %i, path: '%s' -> '%s', ...)\n", __func__, fd,
			  path, real_path);

	return ret;
}
#endif
