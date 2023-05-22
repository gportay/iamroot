/*
 * Copyright 2022-2023 Gaël PORTAY
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

#ifdef __GLIBC__
static int (*sym)(const char *,
		  int (*)(const char *, const struct stat64 *, int, struct FTW *),
	          int, int);

__attribute__((visibility("hidden")))
int next_nftw64(const char *path,
	     int (*fn)(const char *, const struct stat64 *, int, struct FTW *),
	     int nopenfd, int flags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "nftw64");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(path, fn, nopenfd, flags);
}

int nftw64(const char *path,
	   int (*fn)(const char *, const struct stat64 *, int, struct FTW *),
	   int nopenfd, int flags)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_nftw64(buf, fn, nopenfd, flags);

exit:
	__debug("%s(path: '%s' -> '%s') -> %i\n", __func__, path, buf, ret);

	return ret;
}
#endif
