/*
 * Copyright 2021 Gaël PORTAY
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

__attribute__((visibility("hidden")))
int next_fchmodat(int fd, const char *path, mode_t mode, int flags)
{
	int (*sym)(int, const char *, mode_t, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "fchmodat");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, mode, flags);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

int fchmodat(int fd, const char *path, mode_t mode, int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__verbose("%s(fd: %i, path: '%s' -> '%s', mode: 0%03o)\n", __func__,
		  fd, path, real_path, mode);

	return next_fchmodat(fd, real_path, mode, flags);
}
