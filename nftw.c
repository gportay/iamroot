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
int next_nftw(const char *path,
	      int (*fn)(const char *, const struct stat *, int, struct FTW *),
              int nopenfd, int flags)
{
	int (*sym)(const char *,
		   int (*)(const char *, const struct stat *, int, struct FTW *),
		   int, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "nftw");
	if (!sym) {
		__dlperror(__func__);
		return __set_errno(ENOSYS, -1);
	}

	ret = sym(path, fn, nopenfd, flags);
	if (ret == -1)
		__pathperror(path, __func__);

	return ret;
}

int nftw(const char *path,
	 int (*fn)(const char *, const struct stat *, int, struct FTW *),
         int nopenfd, int flags)
{
	char buf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1) {
		__pathperror(path, __func__);
		return -1;
	}

	__debug("%s(path: '%s' -> '%s')\n", __func__, path, buf);

	return next_nftw(buf, fn, nopenfd, flags);
}
