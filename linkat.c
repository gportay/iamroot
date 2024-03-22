/*
 * Copyright 2021-2024 Gaël PORTAY
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

hidden int next_linkat(int olddfd, const char *oldpath, int newdfd,
		       const char *newpath, int atflags)
{
	if (!sym)
		sym = dlsym(RTLD_NEXT, "linkat");

	if (!sym)
		return __dl_set_errno_and_perror(ENOSYS, -1);

	return sym(olddfd, oldpath, newdfd, newpath, atflags);
}

int linkat(int olddfd, const char *oldpath, int newdfd, const char *newpath,
	   int atflags)
{
	char oldbuf[PATH_MAX], newbuf[PATH_MAX];
	int ret = -1;
	ssize_t siz;

	siz = path_resolution(olddfd, oldpath, oldbuf, sizeof(oldbuf), atflags);
	if (siz == -1)
		goto exit;

	siz = path_resolution(newdfd, newpath, newbuf, sizeof(newbuf), 0);
	if (siz == -1)
		goto exit;

	ret = next_linkat(olddfd, oldbuf, newdfd, newbuf, atflags);

exit:
	__debug("%s(olddfd: %i <-> '%s', oldpath: '%s' -> '%s', newdfd: %i <-> '%s', newpath: '%s' -> '%s', atflags: 0x%x) -> %i\n",
		__func__, olddfd, __fpath(olddfd), oldpath, oldbuf, newdfd,
		__fpath2(newdfd), newpath, newbuf, atflags, ret);

	return ret;
}
