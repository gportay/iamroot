/*
 * Copyright 2021-2023 Gaël PORTAY
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
int next_symlinkat(const char *string, int dfd, const char *path)
{
	int (*sym)(const char *, int, const char *);
	int ret;

	sym = dlsym(RTLD_NEXT, "symlinkat");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(string, dfd, path);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int symlinkat(const char *string, int dfd, const char *path)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(dfd, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(string: '%s', dfd: %i <-> '%s', path: '%s' -> '%s')\n",
		__func__, string, dfd, __fpath(dfd), path, buf);

	return next_symlinkat(string, dfd, buf);
}
