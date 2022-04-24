/*
 * Copyright 2021-2022 Gaël PORTAY
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

__attribute__((visibility("hidden")))
int next_linkat(int oldfd, const char *oldpath, int newfd, const char *newpath,
		int flags)
{
	int (*sym)(int, const char *, int, const char *, int);
	int ret;

	sym = dlsym(RTLD_NEXT, "linkat");
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

int linkat(int oldfd, const char *oldpath, int newfd, const char *newpath,
	   int flags)
{
	char oldbuf[PATH_MAX], newbuf[PATH_MAX];

	if (path_resolution(oldfd, oldpath, oldbuf, sizeof(oldbuf),
			    flags) == -1) {
		__pathperror(oldpath, __func__);
		return -1;
	}

	if (path_resolution(newfd, newpath, newbuf, sizeof(newbuf),
			    flags) == -1) {
		__pathperror(newpath, __func__);
		return -1;
	}

	__debug("%s(oldfd: %i, oldpath: '%s' -> '%s', newfd: %i, newpath: '%s' -> '%s', flags: 0x%x)\n",
		__func__, oldfd, oldpath, oldbuf, newfd, newpath, newbuf,
		flags);

	return next_linkat(oldfd, oldbuf, newfd, newbuf, flags);
}
