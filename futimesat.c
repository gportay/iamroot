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

__attribute__((visibility("hidden")))
int next_futimesat(int fd, const char *path, const struct timeval times[2])
{
	int (*sym)(int, const char *, const struct timeval [2]);
	int ret;

	sym = dlsym(RTLD_NEXT, "futimesat");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, times);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

int futimesat(int fd, const char *path, const struct timeval times[2])
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf),
				       AT_SYMLINK_NOFOLLOW);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__verbose("%s(fd: %d, path: '%s' -> '%s')\n", __func__, fd, path,
		  real_path);

	return next_futimesat(fd, real_path, times);
}
