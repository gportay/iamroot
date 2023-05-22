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

static int (*sym)(const char *,
		  int (*)(const char *, const struct stat *, int, struct FTW *),
		  int, int);

__attribute__((visibility("hidden")))
int next_nftw(const char *path,
	      int (*fn)(const char *, const struct stat *, int, struct FTW *),
              int nopenfd, int flags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "nftw");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(path, fn, nopenfd, flags);
}

int nftw(const char *path,
	 int (*fn)(const char *, const struct stat *, int, struct FTW *),
         int nopenfd, int flags)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = next_nftw(buf, fn, nopenfd, flags);

	__debug("%s(path: '%s' -> '%s') -> %i\n", __func__, path, buf, ret);

	return ret;
}
