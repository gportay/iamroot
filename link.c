/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

__attribute__((visibility("hidden")))
int next_link(const char *oldpath, const char *newpath)
{
	int (*sym)(const char *, const char *);
	int ret;

	sym = dlsym(RTLD_NEXT, "link");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(oldpath, newpath);
	if (ret == -1)
		__perror2(oldpath, newpath, __func__);

	return ret;
}

int link(const char *oldpath, const char *newpath)
{
	char oldbuf[PATH_MAX], newbuf[PATH_MAX];
	char *real_oldpath, *real_newpath;

	real_oldpath = path_resolution(oldpath, oldbuf, sizeof(oldbuf),
				       AT_SYMLINK_NOFOLLOW);
	if (!real_oldpath) {
		__perror(oldpath, "path_resolution");
		return -1;
	}

	real_newpath = path_resolution(newpath, newbuf, sizeof(newbuf),
				       AT_SYMLINK_NOFOLLOW);
	if (!real_newpath) {
		__perror(newpath, "path_resolution");
		return -1;
	}

	__debug("%s(oldpath: '%s' -> '%s', newpath: '%s' -> '%s')\n", __func__,
		oldpath, real_oldpath, newpath, real_newpath);

	return next_link(real_oldpath, real_newpath);
}
