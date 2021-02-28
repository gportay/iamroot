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
#include <unistd.h>

#include "iamroot.h"

extern char *fpath_resolutionat(int, const char *, char *, size_t, int);

__attribute__((visibility("hidden")))
int next_unlinkat(int fd, const char *path, int flags)
{
	int (*sym)(int, const char *, int);

	sym = dlsym(RTLD_NEXT, "unlinkat");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	return sym(fd, path, flags);
}

int unlinkat(int fd, const char *path, int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__verbose("%s(fd: %d, path: '%s' -> '%s')\n", __func__, fd, path,
		  real_path);

	return next_unlinkat(fd, real_path, flags);
}
