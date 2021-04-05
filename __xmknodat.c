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

#include "iamroot.h"

extern char *fpath_resolutionat(int, const char *, char *, size_t, int);

__attribute__((visibility("hidden")))
int next___xmknodat(int ver, int fd, const char *path, mode_t mode, dev_t dev)
{
	int (*sym)(int, int, const char *, mode_t, dev_t);

	sym = dlsym(RTLD_NEXT, "__xmknodat");
	if (!sym) {
		errno = ENOSYS;
		return -1;
	}

	return sym(ver, fd, path, mode, dev);
}

int __xmknodat(int ver, int fd, const char *path, mode_t mode, dev_t dev)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__verbose("%s(fd %i, path: '%s' -> '%s')\n", __func__, fd, path,
		  real_path);

	return next___xmknodat(ver, fd, path, mode, dev);
}
