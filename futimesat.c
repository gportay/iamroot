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
#include <sys/time.h>

#include "iamroot.h"

extern char *fpath_resolutionat(int, const char *, char *, size_t, int);

__attribute__((visibility("hidden")))
int next_futimesat(int fd, const char *path, const struct timeval times[2])
{
	int (*sym)(int, const char *, const struct timeval [2]);

	sym = dlsym(RTLD_NEXT, "futimesat");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(fd, path, times);
}

int futimesat(int fd, const char *path, const struct timeval times[2])
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__verbose("%s(fd: %d, path: '%s' -> '%s')\n", __func__, fd, path,
		  real_path);

	return next_futimesat(fd, real_path, times);
}
