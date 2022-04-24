/*
 * Copyright 2021-2022 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <stdio.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_rename(const char *oldpath, const char *newpath)
{
	int (*sym)(const char *, const char *);
	int ret;

	sym = dlsym(RTLD_NEXT, "rename");
	if (!sym) {
		__dlperror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(oldpath, newpath);
	if (ret == -1)
		__pathperror2(oldpath, newpath, __func__);

	return ret;
}

int rename(const char *oldpath, const char *newpath)
{
	char oldbuf[PATH_MAX], newbuf[PATH_MAX];
	ssize_t siz;

	siz = path_resolution(AT_FDCWD, oldpath, oldbuf, sizeof(oldbuf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1) {
		__pathperror(oldpath, __func__);
		return -1;
	}

	siz = path_resolution(AT_FDCWD, newpath, newbuf, sizeof(newbuf),
			      AT_SYMLINK_NOFOLLOW);
	if (siz == -1) {
		__pathperror(newpath, __func__);
		return -1;
	}

	__debug("%s(oldpath: '%s' -> '%s', newpath: '%s' -> '%s')\n", __func__,
		oldpath, oldbuf, newpath, newbuf);

	return next_rename(oldbuf, newbuf);
}
