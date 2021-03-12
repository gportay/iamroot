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

#include <sys/types.h>
#include <dirent.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

__attribute__((visibility("hidden")))
DIR *next_opendir(const char *path)
{
	DIR *(*sym)(const char *);

	sym = dlsym(RTLD_NEXT, "opendir");
	if (!sym) {
		errno = ENOTSUP;
		return NULL;
	}

	return sym(path);
}

DIR *opendir(const char *path)
{
	const char *real_path = path;
	char buf[PATH_MAX];

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return NULL;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next_opendir(real_path);
}
