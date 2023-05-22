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
		  int (*)(const char *, const struct stat *, int),
		  int);

__attribute__((visibility("hidden")))
int next_ftw(const char *path,
	     int (*fn)(const char *, const struct stat *, int),
	     int nopenfd)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "ftw");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(path, fn, nopenfd);
}

int ftw(const char *path, int (*fn)(const char *, const struct stat *, int),
	int nopenfd)
{
	char buf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, path, buf, sizeof(buf), 0);
	if (siz == -1)
		goto exit;

	ret = next_ftw(buf, fn, nopenfd);

exit:
	__debug("%s(path: '%s' -> '%s') -> %i\n", __func__, path, buf, ret);

	return ret;
}
