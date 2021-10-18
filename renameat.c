/*
 * Copyright 2021 Gaël PORTAY
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
int next_renameat(int oldfd, const char *oldpath, int newfd,
		  const char *newpath)
{
	int (*sym)(int, const char *, int, const char *);
	int ret;

	sym = dlsym(RTLD_NEXT, "renameat");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	ret = sym(oldfd, oldpath, newfd, newpath);
	if (ret == -1)
		__perror2(oldpath, newpath, __func__);

	return ret;
}

int renameat(int oldfd, const char *oldpath, int newfd, const char *newpath)
{
	char oldbuf[PATH_MAX], newbuf[PATH_MAX];
	char *real_oldpath, *real_newpath;

	real_oldpath = fpath_resolutionat(oldfd, oldpath, oldbuf,
					  sizeof(oldbuf), AT_SYMLINK_NOFOLLOW);
	if (!real_oldpath) {
		perror("fpath_resolutionat");
		return -1;
	}

	real_newpath = fpath_resolutionat(newfd, newpath, newbuf,
					  sizeof(newbuf), AT_SYMLINK_NOFOLLOW);
	if (!real_newpath) {
		perror("fpath_resolutionat");
		return -1;
	}

	__verbose("%s(oldfd: %i, oldpath: '%s' -> '%s', newfd: %i, newpath: '%s' -> '%s')\n",
		  __func__, oldfd, oldpath, real_oldpath, newfd, newpath,
		  real_newpath);

	return next_renameat(oldfd, real_oldpath, newfd, real_newpath);
}
