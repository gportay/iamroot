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

#include "fpath_resolutionat.h"

#ifdef __GLIBC__
extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next___openat_2(int fd, const char *path, int flags)
{
	int (*sym)(int, const char *, int);

	sym = dlsym(RTLD_NEXT, "__openat_2");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(fd, path, flags);
}

int __openat_2(int fd, const char *path, int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__fprintf(stderr, "%s(fd: %d, path: '%s' -> '%s')\n", __func__, fd,
			  path, real_path);

	return next___openat_2(fd, real_path, flags);
}
#endif
