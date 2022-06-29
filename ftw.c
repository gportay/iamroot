/*
 * Copyright 2021-2022 Gaël PORTAY
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
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

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

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, buf);

	return next_ftw(buf, fn, nopenfd);
}