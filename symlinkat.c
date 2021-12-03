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
#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_symlinkat(const char *string, int fd, const char *path)
{
	int (*sym)(const char *, int, const char *);
	int ret;

	sym = dlsym(RTLD_NEXT, "symlinkat");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(string, fd, path);
	if (ret == -1)
		__perror(path, __func__);

	return ret;
}

int symlinkat(const char *string, int fd, const char *path)
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("fpath_resolutionat");
		return -1;
	}

	__verbose_func("%s(string: '%s', fd: %i, path: '%s' -> '%s')\n",
		       __func__, string, fd, path, real_path);

	return next_symlinkat(string, fd, real_path);
}
