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

#include "iamroot.h"

#ifdef __GLIBC__
__attribute__((visibility("hidden")))
int next_openat64_2(int fd, const char *path, int flags)
{
	int (*sym)(int, const char *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "openat64_2");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(fd, path, flags);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

int openat64_2(int fd, const char *path, int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		__perror(path, "fpath_resolutionat");
		return -1;
	}

	__debug("%s(fd: %d, path: '%s' -> '%s')\n", __func__, fd, path,
		real_path);

	return next_openat64_2(fd, real_path, flags);
}
#endif
