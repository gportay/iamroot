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

#include <dirent.h>

#include "path_resolution.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

int next_scandir(const char *path, struct dirent ***namelist,
		 int (*filter)(const struct dirent *),
		 int (*compar)(const struct dirent **, const struct dirent **))
{
	int (*sym)(const char *, struct dirent ***,
		   int (*)(const struct dirent *),
		   int (*)(const struct dirent **, const struct dirent **));

	sym = dlsym(RTLD_NEXT, "scandir");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(path, namelist, filter, compar);
}

int scandir(const char *path, struct dirent ***namelist,
	    int (*filter)(const struct dirent *),
	    int (*compar)(const struct dirent **, const struct dirent **))
{
	const char *real_path = path;
	char buf[PATH_MAX];

	real_path = path_resolution(path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next_scandir(real_path, namelist, filter, compar);
}
