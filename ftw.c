/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <ftw.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_ftw(const char *path,
	     int (*fn)(const char *, const struct stat *, int),
	     int nopenfd)
{
	int (*sym)(const char *,
		   int (*)(const char *, const struct stat *, int), int);
	int ret;

	sym = dlsym(RTLD_NEXT, "ftw");
	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	ret = sym(path, fn, nopenfd);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int ftw(const char *path, int (*fn)(const char *, const struct stat *, int),
	int nopenfd)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = next_ftw(buf, fn, nopenfd);

	__debug("%s(path: '%s' -> '%s') -> %i\n", __func__, path, buf, ret);

	return ret;
}
