/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_symlinkat(const char *string, int fd, const char *path)
{
	int (*sym)(const char *, int, const char *);
	int ret;

	sym = dlsym(RTLD_NEXT, "symlinkat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(string, fd, path);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int symlinkat(const char *string, int fd, const char *path)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(fd, path, buf, sizeof(buf), 0);
	if (!real_path) {
		__pathperror(path, "path_resolution");
		return -1;
	}

	__debug("%s(string: '%s', fd: %i, path: '%s' -> '%s')\n", __func__,
		string, fd, path, real_path);

	return next_symlinkat(string, fd, real_path);
}
