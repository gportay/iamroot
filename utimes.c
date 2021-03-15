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

#include <sys/time.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
int next_utimes(const char *path, const struct timeval times[2])
{
	int (*sym)(const char *, const struct timeval[2]);

	sym = dlsym(RTLD_NEXT, "utimes");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, times);
}

int utimes(const char *path, const struct timeval times[2])
{
	char buf[PATH_MAX];
	char *real_path;

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next_utimes(real_path, times);
}
