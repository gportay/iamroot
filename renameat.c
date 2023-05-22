/*
 * Copyright 2021-2023 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <stdio.h>

#include "iamroot.h"

static int (*sym)(int, const char *, int, const char *);

__attribute__((visibility("hidden")))
int next_renameat(int olddfd, const char *oldpath, int newdfd,
		  const char *newpath)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "renameat");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(olddfd, oldpath, newdfd, newpath);
}

int renameat(int olddfd, const char *oldpath, int newdfd, const char *newpath)
{
	char oldbuf[PATH_MAX], newbuf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(olddfd, oldpath, oldbuf, sizeof(oldbuf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		return __path_resolution_perror(oldpath, -1);

	siz = path_resolution(newdfd, newpath, newbuf, sizeof(newbuf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1)
		return __path_resolution_perror(newpath, -1);

	ret = next_renameat(olddfd, oldbuf, newdfd, newbuf);

	__debug("%s(olddfd: %i <-> '%s', oldpath: '%s' -> '%s', newdfd: %i <-> '%s', newpath: '%s' -> '%s') -> %i\n",
		__func__, olddfd, __fpath(olddfd), oldpath, oldbuf, newdfd,
		__fpath2(newdfd), newpath, newbuf, ret);

	return ret;
}
