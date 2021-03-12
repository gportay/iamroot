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
#include <sys/time.h>

#include "fpath_resolutionat.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

int next_futimesat(int fd, const char *path, const struct timeval times[2])
{
	int (*sym)(int, const char *, const struct timeval [2]);

	sym = dlsym(RTLD_NEXT, "futimesat");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(fd, path, times);
}

int futimesat(int fd, const char *path, const struct timeval times[2])
{
	const char *real_path = path;
	char buf[PATH_MAX];

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__fprintf(stderr, "%s(fd: %d, path: '%s' -> '%s')\n", __func__, fd,
			  path, real_path);

	return next_futimesat(fd, real_path, times);
}
