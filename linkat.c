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

static int (*sym)(int, const char *, int, const char *, int);

__attribute__((visibility("hidden")))
int next_linkat(int olddfd, const char *oldpath, int newdfd,
		const char *newpath, int atflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "linkat");

	if (!sym)
		return __dl_set_errno(ENOSYS, -1);

	return sym(olddfd, oldpath, newdfd, newpath, atflags);
}

int linkat(int olddfd, const char *oldpath, int newdfd, const char *newpath,
	   int atflags)
{
	char oldbuf[PATH_MAX], newbuf[PATH_MAX];
	ssize_t siz;
	int ret;

	siz = path_resolution(olddfd, oldpath, oldbuf, sizeof(oldbuf), atflags);
	if (siz == -1)
		return __path_resolution_perror(oldpath, -1);

	siz = path_resolution(newdfd, newpath, newbuf, sizeof(newbuf), 0);
	if (siz == -1)
		return __path_resolution_perror(newpath, -1);

	ret = next_linkat(olddfd, oldbuf, newdfd, newbuf, atflags);

	__debug("%s(olddfd: %i <-> '%s', oldpath: '%s' -> '%s', newdfd: %i <-> '%s', newpath: '%s' -> '%s', atflags: 0x%x) -> %i\n",
		__func__, olddfd, __fpath(olddfd), oldpath, oldbuf, newdfd,
		__fpath2(newdfd), newpath, newbuf, atflags, ret);

	return ret;
}
