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

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
ssize_t next_readlinkat(int fd, const char *path, char *buf, size_t bufsize)
{
	ssize_t (*sym)(int, const char *, char *, size_t);

	sym = dlsym(RTLD_NEXT, "readlinkat");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(fd, path, buf, bufsize);
}

ssize_t readlinkat(int fd, const char *path, char *buf, size_t bufsize)
{
	char tmp[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, tmp, sizeof(tmp), 0);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__fprintf(stderr, "%s(fd: %i, path: '%s' -> '%s', ...)\n", __func__, fd,
			  path, real_path);

	return next_readlinkat(fd, real_path, buf, bufsize);
}
