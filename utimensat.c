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
#include <sys/time.h>

#include "iamroot.h"

extern char *fpath_resolutionat(int, const char *, char *, size_t, int);

__attribute__((visibility("hidden")))
int next_utimensat(int fd, const char *path, const struct timespec times[2],
		   int flags)
{
	int (*sym)(int, const char *, const struct timespec[2], int);

	sym = dlsym(RTLD_NEXT, "utimensat");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	return sym(fd, path, times, flags);
}

int utimensat(int fd, const char *path, const struct timespec times[2],
	      int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__verbose("%s(fd: %i, path: '%s' -> '%s')\n", __func__, fd, path,
		  real_path);

	return next_utimensat(fd, real_path, times, flags);
}
