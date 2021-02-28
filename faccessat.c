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
int next_faccessat(int fd, const char *path, int mode, int flags)
{
	int (*sym)(int, const char *, int, int);

	sym = dlsym(RTLD_NEXT, "faccessat");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	return sym(fd, path, mode, flags);
}

int faccessat(int fd, const char *path, int mode, int flags)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), flags);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__verbose("%s(fd: %i, path: '%s' -> '%s')\n", __func__, fd, path,
		  real_path);

	return next_faccessat(fd, real_path, mode, flags);
}
