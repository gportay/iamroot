/*
 * Copyright 2021 Gaël PORTAY
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <dlfcn.h>

#include <unistd.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...);

ssize_t readlink(const char *path, char *buf, size_t bufsize)
{
	ssize_t (*realsym)(const char *, char *, size_t);
	const char *real_path;
	char tmp[PATH_MAX];

	real_path = path_resolution(path, tmp, sizeof(tmp));
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s', ...)\n", __func__, path,
			  real_path);

	realsym = dlsym(RTLD_NEXT, __func__);
	return realsym(real_path, buf, bufsize);
}
