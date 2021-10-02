/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <sys/stat.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next___xmknod(int ver, const char *path, mode_t mode, dev_t *dev)
{
	int (*sym)(int, const char *, mode_t, dev_t *);

	sym = dlsym(RTLD_NEXT, "__xmknod");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	return sym(ver, path, mode, dev);
}

int __xmknod(int ver, const char *path, mode_t mode, dev_t *dev)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__verbose("%s(path: '%s' -> '%s')\n", __func__, path, real_path);

	return next___xmknod(ver, path, mode, dev);
}
