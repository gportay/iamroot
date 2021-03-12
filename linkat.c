/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <dlfcn.h>

#include <fcntl.h>
#include <unistd.h>

#include "fpath_resolutionat.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next_linkat(int oldfd, const char *oldpath, int newfd, const char *newpath,
		int flags)
{
	int (*sym)(int, const char *, int, const char *, int);

	sym = dlsym(RTLD_NEXT, "linkat");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(oldfd, oldpath, newfd, newpath, flags);
}

int linkat(int oldfd, const char *oldpath, int newfd, const char *newpath,
	     int flags)
{
	const char *real_oldpath, *real_newpath;
	char oldbuf[PATH_MAX], newbuf[PATH_MAX];

	real_oldpath = fpath_resolutionat(oldfd, oldpath, oldbuf,
					  sizeof(oldbuf), flags);
	if (!real_oldpath) {
		perror("fpath_resolutionat");
		return -1;
	}

	real_newpath = fpath_resolutionat(newfd, newpath, newbuf,
					  sizeof(newbuf), flags);
	if (!real_newpath) {
		perror("fpath_resolutionat");
		return -1;
	}

	__fprintf(stderr, "%s(oldfd: %i, oldpath: '%s' -> '%s', newfd: %i, newpath: '%s' -> '%s')\n",
			  __func__, oldfd, oldpath, real_oldpath, newfd,
			  newpath, real_newpath);

	return next_linkat(oldfd, real_oldpath, newfd, real_newpath, flags);
}
