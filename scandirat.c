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
#include <dirent.h>

#include "fpath_resolutionat.h"

extern int __fprintf(FILE *, const char *, ...) __attribute__ ((format(printf,2,3)));

int next_scandirat(int fd, const char *path, struct dirent ***namelist,
		  int (*filter)(const struct dirent *),
		  int (*compar)(const struct dirent **, const struct dirent **))
{
	int (*sym)(int, const char *, struct dirent ***,
		   int (*)(const struct dirent *),
		   int (*)(const struct dirent **, const struct dirent **));

	sym = dlsym(RTLD_NEXT, "scandirat");
	if (!sym) {
		errno = ENOTSUP;
		return -1;
	}

	return sym(fd, path, namelist, filter, compar);
}

int scandirat(int fd, const char *path, struct dirent ***namelist,
	      int (*filter)(const struct dirent *),
	      int (*compar)(const struct dirent **, const struct dirent **))
{
	const char *real_path = path;
	char buf[PATH_MAX];

	real_path = fpath_resolutionat(fd, path, buf, sizeof(buf), 0);
	if (!real_path) {
		perror("path_resolution");
		return -1;
	}

	__fprintf(stderr, "%s(path: '%s' -> '%s')\n", __func__, path,
			  real_path);

	return next_scandirat(fd, real_path, namelist, filter, compar);
}
