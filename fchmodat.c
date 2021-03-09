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

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

int next_fchmodat(int fd, const char *path, mode_t mode, int flags)
{
	int (*sym)(int, const char *, mode_t, int);

	sym = dlsym(RTLD_NEXT, "fchmodat");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(fd, path, mode, flags);
}

int fchmodat(int fd, const char *path, mode_t mode, int flags)
{
	const char *real_path;
	char buf[PATH_MAX];

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__fprintf(stderr, "%s(fd: %i, path: '%s' -> '%s')\n", __func__, fd,
			  path, real_path);

	return next_fchmodat(fd, real_path, mode, flags);
}
