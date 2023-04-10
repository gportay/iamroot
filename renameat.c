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

__attribute__((visibility("hidden")))
int next_renameat(int olddfd, const char *oldpath, int newdfd,
		  const char *newpath)
{
	int (*sym)(int, const char *, int, const char *);
	int ret;

	sym = dlsym(RTLD_NEXT, "renameat");
	if (!sym) {
		__dlperror(__func__);
		return __set_errno(ENOSYS, -1);
	}

	ret = sym(olddfd, oldpath, newdfd, newpath);
	if (ret == -1)
		__pathperror2(oldpath, newpath, __func__);

	return ret;
}

int renameat(int olddfd, const char *oldpath, int newdfd, const char *newpath)
{
	char oldbuf[PATH_MAX], newbuf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(olddfd, oldpath, oldbuf, sizeof(oldbuf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1) {
		__pathperror(oldpath, __func__);
		return -1;
	}

	siz = path_resolution(newdfd, newpath, newbuf, sizeof(newbuf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1) {
		__pathperror(newpath, __func__);
		return -1;
	}

	__debug("%s(olddfd: %i <-> '%s', oldpath: '%s' -> '%s', newdfd: %i <-> '%s', newpath: '%s' -> '%s')\n",
		__func__, olddfd, __fpath(olddfd), oldpath, oldbuf, newdfd,
		__fpath2(newdfd), newpath, newbuf);

	return next_renameat(olddfd, oldbuf, newdfd, newbuf);
}
