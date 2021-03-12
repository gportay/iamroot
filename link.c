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

#include <unistd.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next_link(const char *oldpath, const char *newpath)
{
	int (*sym)(const char *, const char *);

	sym = dlsym(RTLD_NEXT, "link");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(oldpath, newpath);
}

int link(const char *oldpath, const char *newpath)
{
	const char *real_oldpath, *real_newpath;
	char oldbuf[PATH_MAX], newbuf[PATH_MAX];

	real_oldpath = path_resolution(oldpath, oldbuf, sizeof(oldbuf), 0);
	if (!real_oldpath) {
		perror("path_resolution");
		return -1;
	}

	real_newpath = path_resolution(newpath, newbuf, sizeof(newbuf), 0);
	if (!real_newpath) {
		perror("path_resolution");
		return -1;
	}

	__fprintf(stderr, "%s(oldpath: '%s' -> '%s', newpath: '%s' -> '%s')\n",
			  __func__, oldpath, real_oldpath, newpath,
			  real_newpath);

	return next_link(real_oldpath, real_newpath);
}
