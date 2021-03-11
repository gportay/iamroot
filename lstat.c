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
#include <sys/stat.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));
extern int lrootstat(const char *, struct stat *);

int next_lstat(const char *path, struct stat *statbuf)
{
	int (*sym)(const char *, struct stat *);

	sym = dlsym(RTLD_NEXT, "lstat");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, statbuf);
}

int lstat(const char *path, struct stat *statbuf)
{
	const char *real_path;
	char buf[PATH_MAX];
	int ret;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	ret = lrootstat(real_path, statbuf);

	__fprintf(stderr, "%s(path: '%s' -> '%s', ...)\n", __func__, path,
			  real_path);

	return ret;
}
