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
int next_renameat2(int oldfd, const char *oldpath, int newfd, const char *newpath,
		   unsigned int flags)
{
	int (*sym)(int, const char *, int, const char *, unsigned int);
	int ret;

	sym = dlsym(RTLD_NEXT, "renameat2");
	if (!sym) {
		__dl_perror(__func__);
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
	char *real_oldpath, *real_newpath;

	real_oldpath = fpath_resolutionat(oldfd, oldpath, oldbuf,
					  sizeof(oldbuf), (int)flags);
	if (!real_oldpath) {
		__pathperror(oldpath, "fpath_resolutionat");
		return -1;
	}

	real_newpath = fpath_resolutionat(newfd, newpath, newbuf,
					  sizeof(newbuf), (int)flags);
	if (!real_newpath) {
		__pathperror(newpath, "fpath_resolutionat");
		return -1;
	}

	__debug("%s(oldfd: %i, oldpath: '%s' -> '%s', newfd: %i, newpath: '%s' -> '%s')\n",
		__func__, oldfd, oldpath, real_oldpath, newfd, newpath,
		real_newpath);

	return next_renameat2(oldfd, real_oldpath, newfd, real_newpath, flags);
}
