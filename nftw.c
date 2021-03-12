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

#include <ftw.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next_nftw(const char *path,
	      int (*fn)(const char *, const struct stat *, int, struct FTW *),
              int nopenfd, int flags)
{
	int (*sym)(const char *,
	          int (*)(const char *, const struct stat *, int, struct FTW *),
		  int, int);

	sym = dlsym(RTLD_NEXT, "nftw");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, fn, nopenfd, flags);
}

int nftw(const char *path,
	 int (*fn)(const char *, const struct stat *, int, struct FTW *),
         int nopenfd, int flags)
{
	const char *real_path;
	char buf[PATH_MAX];

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next_nftw(real_path, fn, nopenfd, flags);
}
