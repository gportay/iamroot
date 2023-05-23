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
#include <sys/stat.h>
#include <sys/time.h>

#include "iamroot.h"

static int (*sym)(int, const char *, const struct timespec[2], int);

__attribute__((visibility("hidden")))
int next_utimensat(int dfd, const char *path, const struct timespec times[2],
		   int atflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "utimensat");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(dfd, path, times, atflags);
}

int utimensat(int dfd, const char *path, const struct timespec times[2],
	      int atflags)
{
	char buf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(dfd, path, buf, sizeof(buf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		return __path_resolution_perror(path, -1);

	ret = next_utimensat(dfd, buf, times, atflags);

	__debug("%s(dfd: %i <-> '%s', path: '%s' -> '%s', ..., atflags: 0x%x) -> %i\n",
		__func__, dfd, __fpath(dfd), path, buf, atflags, ret);

	return ret;
}

#ifdef __GLIBC__
weak_alias(utimensat, __utimensat64);
#endif
