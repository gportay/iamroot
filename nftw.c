/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <string.h>
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
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(path, fn, nopenfd, flags);
}

static int (*__fn)(const char *, const struct stat *, int, struct FTW *);
static int __nftw_callback(const char *path, const struct stat *statbuf,
			   int flags, struct FTW *ftwbuf)
{
	char buf[PATH_MAX];

	__strncpy(buf, path);
	__striprootdir(buf);
	
	__debug("%s(path: '%s' -> '%s', ...)\n", __func__, path, buf);

	if (!__fn)
		return -1;

	return __fn(buf, statbuf, flags, ftwbuf);
}

int nftw(const char *path,
	 int (*fn)(const char *, const struct stat *, int, struct FTW *),
         int nopenfd, int flags)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	__fn = fn;
	ret = next_nftw(buf, __nftw_callback, nopenfd, flags);
	__fn = NULL;

exit:
	__debug("%s(path: '%s' -> '%s') -> %i\n", __func__, path, buf, ret);

	return ret;
}
