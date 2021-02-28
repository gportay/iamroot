/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <unistd.h>

#include "iamroot.h"

extern char *path_resolution(const char *, char *, size_t, int);

__attribute__((visibility("hidden")))
int next_link(const char *oldpath, const char *newpath)
{
	int (*sym)(const char *, const char *);

	sym = dlsym(RTLD_NEXT, "link");
	if (!sym) {
		__dl_perror(__func__);
		errno = ENOSYS;
		return -1;
	}

	return sym(oldpath, newpath);
}

int link(const char *oldpath, const char *newpath)
{
	char oldbuf[PATH_MAX], newbuf[PATH_MAX];
	char *real_oldpath, *real_newpath;

	real_oldpath = path_resolution(oldpath, oldbuf, sizeof(oldbuf),
				       AT_SYMLINK_NOFOLLOW);
	if (!real_oldpath) {
		perror("path_resolution");
		return -1;
	}

	real_newpath = path_resolution(newpath, newbuf, sizeof(newbuf),
				       AT_SYMLINK_NOFOLLOW);
	if (!real_newpath) {
		perror("path_resolution");
		return -1;
	}

	__verbose("%s(oldpath: '%s' -> '%s', newpath: '%s' -> '%s')\n",
		  __func__, oldpath, real_oldpath, newpath, real_newpath);

	return next_link(real_oldpath, real_newpath);
}
