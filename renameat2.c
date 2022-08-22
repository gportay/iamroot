/*
 * Copyright 2021-2022 Gaël PORTAY
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
int next_renameat2(int oldfd, const char *oldpath, int newfd,
		   const char *newpath, unsigned int flags)
{
	int (*sym)(int, const char *, int, const char *, unsigned int);
	int ret;

	sym = dlsym(RTLD_NEXT, "renameat2");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(oldfd, oldpath, newfd, newpath, flags);
	if (ret == -1)
		__pathperror2(oldpath, newpath, __func__);

	return ret;
}

int renameat2(int oldfd, const char *oldpath, int newfd, const char *newpath,
	      unsigned int flags)
{
	char oldbuf[PATH_MAX], newbuf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(oldfd, oldpath, oldbuf, sizeof(oldbuf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1) {
		__pathperror(oldpath, __func__);
		return -1;
	}

	siz = path_resolution(newfd, newpath, newbuf, sizeof(newbuf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1) {
		__pathperror(newpath, __func__);
		return -1;
	}

	__debug("%s(oldfd: %i, oldpath: '%s' -> '%s', newfd: %i, newpath: '%s' -> '%s', flags: )\n",
		__func__, oldfd, oldpath, oldbuf, newfd, newpath, newbuf);

	return next_renameat2(oldfd, oldbuf, newfd, newbuf, flags);
}
